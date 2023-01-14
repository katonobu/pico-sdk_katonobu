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

#ifndef _PICO_TINYUSB_ARCH_ARCH_POLL_H
#define _PICO_TINYUSB_ARCH_ARCH_POLL_H

#include "pico/tinyusb_arch/arch_common.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TINYUSB_THREAD_ENTER
#define TINYUSB_THREAD_EXIT
#ifndef NDEBUG

void tinyusb_thread_check(void);

#define tinyusb_arch_lwip_check() tinyusb_thread_check()
#define TINYUSB_THREAD_LOCK_CHECK tinyusb_arch_lwip_check();
#else
#define tinyusb_arch_lwip_check() ((void)0)
#define TINYUSB_THREAD_LOCK_CHECK
#endif

#define TINYUSB_SDPCM_SEND_COMMON_WAIT tinyusb_poll_required = true;
#define TINYUSB_DO_IOCTL_WAIT tinyusb_poll_required = true;

#define tinyusb_delay_ms sleep_ms
#define tinyusb_delay_us sleep_us

void tinyusb_schedule_internal_poll_dispatch(void (*func)(void));

void tinyusb_post_poll_hook(void);

extern bool tinyusb_poll_required;

#define TINYUSB_POST_POLL_HOOK tinyusb_post_poll_hook();
#endif

#ifndef DOXYGEN_GENERATION // multiple definitions in separate headers seems to confused doxygen
#define tinyusb_arch_lwip_begin() ((void)0)
#define tinyusb_arch_lwip_end() ((void)0)

static inline int tinyusb_arch_lwip_protect(int (*func)(void *param), void *param) {
    return func(param);
}

#ifdef __cplusplus
}
#endif

#endif
