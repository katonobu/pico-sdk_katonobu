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

#ifndef _PICO_TINYUSB_ARCH_ARCH_COMMON_H
#define _PICO_TINYUSB_ARCH_ARCH_COMMON_H

#include "pico.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "pico/error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Note, these are negated, because tinyusb_driver negates them before returning!
#define TINYUSB_EPERM            (-PICO_ERROR_NOT_PERMITTED) // Operation not permitted
#define TINYUSB_EIO              (-PICO_ERROR_IO) // I/O error
#define TINYUSB_EINVAL           (-PICO_ERROR_INVALID_ARG) // Invalid argument
#define TINYUSB_ETIMEDOUT        (-PICO_ERROR_TIMEOUT) // Connection timed out

#define tinyusb_hal_pin_obj_t uint

// get the number of elements in a fixed-size array
#define TINYUSB_ARRAY_SIZE(a) count_of(a)

static inline uint32_t tinyusb_hal_ticks_us(void) {
    return time_us_32();
}

static inline uint32_t tinyusb_hal_ticks_ms(void) {
    return to_ms_since_boot(get_absolute_time());
}

void tinyusb_hal_get_mac(int idx, uint8_t buf[6]);

void tinyusb_hal_generate_laa_mac(int idx, uint8_t buf[6]);

#ifdef __cplusplus
}
#endif

#endif

