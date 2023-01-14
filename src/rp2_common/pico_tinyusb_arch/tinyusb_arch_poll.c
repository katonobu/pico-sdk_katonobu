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

#include <stdio.h>
#include "pico/sem.h"
#include "pico/tinyusb_arch.h"
#include "tinyusb_stats.h"

#include "bsp/board.h"
#include "tusb.h"


#if PICO_TINYUSB_ARCH_POLL
#include <lwip/init.h>
#include "lwip/timeouts.h"

#if TINYUSB_LWIP && !NO_SYS
#error PICO_TINYUSB_ARCH_POLL requires lwIP NO_SYS=1
#endif

#ifndef NDEBUG
uint8_t tinyusb_core_num;
#endif

bool tinyusb_poll_required;

void tinyusb_post_poll_hook(void) {
    // ToDo:check needed or not
    // gpio_set_irq_enabled(TINYUSB_PIN_WL_HOST_WAKE, GPIO_IRQ_LEVEL_HIGH, true); 
}

int tinyusb_arch_init(void) {
#ifndef NDEBUG
    tinyusb_core_num = (uint8_t)get_core_num();
#endif
    /* initialize TinyUSB */
    board_init();
    tusb_init();

//    tinyusb_init(&tinyusb_state);
    static bool done_lwip_init;
    if (!done_lwip_init) {
        lwip_init();
        done_lwip_init = true;
    }
    return 0;
}

void tinyusb_arch_deinit(void) {
    // nothing to do
//    tinyusb_deinit(&tinyusb_state);
}


void tinyusb_schedule_internal_poll_dispatch(__unused void (*func)(void)) {
    tinyusb_poll_required = true;
}

void tinyusb_arch_poll(void)
{
    TINYUSB_STAT_INC(LWIP_RUN_COUNT);
    sys_check_timeouts();
    if (tinyusb_poll) {
        if (tinyusb_sleep > 0) {
            // todo check this; but we don't want to advance too quickly
            static absolute_time_t last_poll_time;
            absolute_time_t current = get_absolute_time();
            if (absolute_time_diff_us(last_poll_time, current) > 1000) {
                if (--tinyusb_sleep == 0) {
                    tinyusb_poll_required = 1;
                }
                last_poll_time = current;
            }
        }
        // todo graham i removed this because otherwise polling can do nothing during connect.
        //  in the polling only case, the caller is responsible for throttling how often they call anyway.
        //  The alternative would be to have the call to this function from the init set the poll_required flag first
//        if (tinyusb_poll_required) {
            tinyusb_poll();
//            tinyusb_poll_required = false;
//        }
    }
}

#ifndef NDEBUG
void tinyusb_thread_check() {
    if (__get_current_exception() || get_core_num() != tinyusb_core_num) {
        panic("tinyusb_thread_lock_check failed");
    }
}
#endif

#endif