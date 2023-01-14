// MIT License
//
// Copyright (c) 2023 Nobuo Kato (katonobu4649@gmail.com)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#if 0
#include <stdio.h>

#include "pico/tinyusb_arch.h"
#include "pico/mutex.h"
#include "pico/sem.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"

#include "tinyusb_stats.h"

#if TINYUSB_LWIP
#include <lwip/init.h>
#include "lwip/timeouts.h"
#endif

// note same code
#if PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND

#if PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND && TINYUSB_LWIP && !NO_SYS
#error PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND requires lwIP NO_SYS=1
#endif
#if PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND && TINYUSB_LWIP && MEM_LIBC_MALLOC
#error MEM_LIBC_MALLOC is incompatible with PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND
#endif
// todo right now we are now always doing a tinyusb_dispatch along with a lwip one when hopping cores in low_prio_irq_schedule_dispatch

#ifndef TINYUSB_SLEEP_CHECK_MS
#define TINYUSB_SLEEP_CHECK_MS 50 // How often to run lwip callback
#endif
static alarm_id_t periodic_alarm = -1;

static inline uint recursive_mutex_enter_count(recursive_mutex_t *mutex) {
    return mutex->enter_count;
}

static inline lock_owner_id_t recursive_mutex_owner(recursive_mutex_t *mutex) {
    return mutex->owner;
}

#define TINYUSB_GPIO_IRQ_HANDLER_PRIORITY 0x40

enum {
    TINYUSB_DISPATCH_SLOT_TINYUSB = 0,
    TINYUSB_DISPATCH_SLOT_ADAPTER,
    TINYUSB_DISPATCH_SLOT_ENUM_COUNT
};
#ifndef TINYUSB_DISPATCH_SLOT_COUNT
#define TINYUSB_DISPATCH_SLOT_COUNT TINYUSB_DISPATCH_SLOT_ENUM_COUNT
#endif

typedef void (*low_prio_irq_dispatch_t)(void);
static void low_prio_irq_schedule_dispatch(size_t slot, low_prio_irq_dispatch_t f);

static uint8_t tinyusb_core_num;
#ifndef NDEBUG
static bool in_low_priority_irq;
#endif
static uint8_t low_priority_irq_num;
static bool low_priority_irq_missed;
static low_prio_irq_dispatch_t low_priority_irq_dispatch_slots[TINYUSB_DISPATCH_SLOT_COUNT];
static recursive_mutex_t tinyusb_mutex;
semaphore_t tinyusb_irq_sem;

// Called in low priority pendsv interrupt only to do lwip processing and check tinyusb sleep
static void periodic_worker(void)
{
#if TINYUSB_USE_STATS
    static uint32_t counter;
    if (counter++ % (30000 / LWIP_SYS_CHECK_MS) == 0) {
        tinyusb_dump_stats();
    }
#endif

    TINYUSB_STAT_INC(LWIP_RUN_COUNT);
#if TINYUSB_LWIP
    sys_check_timeouts();
#endif
    if (tinyusb_poll) {
        if (tinyusb_sleep > 0) {
            if (--tinyusb_sleep == 0) {
                low_prio_irq_schedule_dispatch(TINYUSB_DISPATCH_SLOT_TINYUSB, tinyusb_poll);
            }
        }
    }
}

// Regular callback to get lwip to check for timeouts
static int64_t periodic_alarm_handler(__unused alarm_id_t id, __unused void *user_data)
{
    // Do lwip processing in low priority pendsv interrupt
    low_prio_irq_schedule_dispatch(TINYUSB_DISPATCH_SLOT_ADAPTER, periodic_worker);
    return TINYUSB_SLEEP_CHECK_MS * 1000;
}

void tinyusb_await_background_or_timeout_us(uint32_t timeout_us) {
    // if we are called from within an IRQ, then don't wait (we are only ever called in a polling loop)
    if (!__get_current_exception()) {
        sem_acquire_timeout_us(&tinyusb_irq_sem, timeout_us);
    }
}

int tinyusb_arch_init(void) {
    tinyusb_core_num = get_core_num();
    recursive_mutex_init(&tinyusb_mutex);
    /* initialize TinyUSB */
    board_init();
    tusb_init();
//    tinyusb_init(&tinyusb_state);
    sem_init(&tinyusb_irq_sem, 0, 1);

    // Start regular lwip callback to handle timeouts
    periodic_alarm = add_alarm_in_us(TINYUSB_SLEEP_CHECK_MS * 1000, periodic_alarm_handler, NULL, true);
    if (periodic_alarm < 0) {
        return PICO_ERROR_GENERIC;
    }

#if TINYUSB_LWIP
    lwip_init();
#endif
    return PICO_OK;
}

void tinyusb_arch_deinit(void) {
    if (periodic_alarm >= 0) {
        cancel_alarm(periodic_alarm);
        periodic_alarm = -1;
    }
}

void tinyusb_post_poll_hook(void) {
    // ToDo:check needed or not
//    gpio_set_irq_enabled(TINYUSB_PIN_WL_HOST_WAKE, GPIO_IRQ_LEVEL_HIGH, true);
}

// This is called in the gpio and low_prio_irq interrupts and on either core
static void low_prio_irq_schedule_dispatch(size_t slot, low_prio_irq_dispatch_t f) {
    assert(slot < count_of(low_priority_irq_dispatch_slots));
    low_priority_irq_dispatch_slots[slot] = f;
    if (tinyusb_core_num == get_core_num()) {
        //on same core, can dispatch directly
        irq_set_pending(low_priority_irq_num);
    } else {
        // on wrong core, so force via GPIO IRQ which itself calls this method for the TINYUSB slot.
        // since the TINYUSB slot always uses the same function, this is fine with the addition of an
        // extra (but harmless) TINYUSB slot call when another SLOT is invoked.
        // We could do better, but would have to track why the IRQ was called.
        io_irq_ctrl_hw_t *irq_ctrl_base = tinyusb_core_num ?
                                          &iobank0_hw->proc1_irq_ctrl : &iobank0_hw->proc0_irq_ctrl;
        hw_set_bits(&irq_ctrl_base->intf[TINYUSB_PIN_WL_HOST_WAKE/8], GPIO_IRQ_LEVEL_HIGH << (4 * (TINYUSB_PIN_WL_HOST_WAKE & 7)));
    }
}

void tinyusb_schedule_internal_poll_dispatch(void (*func)(void)) {
    low_prio_irq_schedule_dispatch(TINYUSB_DISPATCH_SLOT_TINYUSB, func);
}

// Prevent background processing in pensv and access by the other core
// These methods are called in pensv context and on either core
// They can be called recursively
void tinyusb_thread_enter(void) {
    // Lock the other core and stop low_prio_irq running
    recursive_mutex_enter_blocking(&tinyusb_mutex);
}

#ifndef NDEBUG
void tinyusb_thread_lock_check(void) {
    // Lock the other core and stop low_prio_irq running
    if (recursive_mutex_enter_count(&tinyusb_mutex) < 1 || recursive_mutex_owner(&tinyusb_mutex) != lock_get_caller_owner_id()) {
        panic("tinyusb_thread_lock_check failed");
    }
}
#endif

// Re-enable background processing
void tinyusb_thread_exit(void) {
    // Run low_prio_irq if needed
    if (1 == recursive_mutex_enter_count(&tinyusb_mutex)) {
        // note the outer release of the mutex is not via tinyusb_exit in the low_priority_irq case (it is a direct mutex exit)
        assert(!in_low_priority_irq);
//        if (low_priority_irq_missed) {
//            low_priority_irq_missed = false;
            if (low_priority_irq_dispatch_slots[TINYUSB_DISPATCH_SLOT_TINYUSB]) {
                low_prio_irq_schedule_dispatch(TINYUSB_DISPATCH_SLOT_TINYUSB, tinyusb_poll);
            }
//        }
    }
    recursive_mutex_exit(&tinyusb_mutex);
}


static void tinyusb_delay_until(absolute_time_t until) {
    // sleep can be called in IRQs, so there's not much we can do there
    if (__get_current_exception()) {
        busy_wait_until(until);
    } else {
        sleep_until(until);
    }
}

void tinyusb_delay_ms(uint32_t ms) {
    tinyusb_delay_until(make_timeout_time_ms(ms));
}

void tinyusb_delay_us(uint32_t us) {
    tinyusb_delay_until(make_timeout_time_us(us));
}

void tinyusb_arch_poll() {
    // should not be necessary
//    if (tinyusb_poll) {
//        low_prio_irq_schedule_dispatch(TINYUSB_DISPATCH_SLOT_TINYUSB, tinyusb_poll);
//    }
}

#if !TINYUSB_LWIP
static void no_lwip_fail() {
    panic("You cannot use IP with pico_tinyusb_arch_none");
}
void tinyusb_cb_tcpip_init(tinyusb_t *self, int itf) {
}
void tinyusb_cb_tcpip_deinit(tinyusb_t *self, int itf) {
}
void tinyusb_cb_tcpip_set_link_up(tinyusb_t *self, int itf) {
    no_lwip_fail();
}
void tinyusb_cb_tcpip_set_link_down(tinyusb_t *self, int itf) {
    no_lwip_fail();
}
void tinyusb_cb_process_ethernet(void *cb_data, int itf, size_t len, const uint8_t *buf) {
    no_lwip_fail();
}
#endif

#endif
#endif