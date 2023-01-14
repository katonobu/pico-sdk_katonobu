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

// This header is included by tinyusb_driver to setup its environment

#ifndef _TINYUSB_CONFIGPORT_H
#define _TINYUSB_CONFIGPORT_H

#include "pico.h"

#ifdef PICO_TINYUSB_ARCH_HEADER
#include __XSTRING(PICO_TINYUSB_ARCH_HEADER)
#else
#if PICO_TINYUSB_ARCH_POLL
#include "pico/tinyusb_arch/arch_poll.h"
#elif PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND
#include "pico/tinyusb_arch/arch_threadsafe_background.h"
#elif PICO_TINYUSB_ARCH_FREERTOS
#include "pico/tinyusb_arch/arch_freertos.h"
#else
#error must specify support pico_tinyusb_arch architecture type or set PICO_TINYUSB_ARCH_HEADER
#endif
#endif

#ifndef TINYUSB_HOST_NAME
#define TINYUSB_HOST_NAME "PicoUSB"
#endif

#endif

#ifndef TINYUSB_LOGIC_DEBUG
#define TINYUSB_LOGIC_DEBUG 0
#endif

#ifndef TINYUSB_USE_OTP_MAC
#define TINYUSB_USE_OTP_MAC 1
#endif

#ifndef TINYUSB_NO_NETUTILS
#define TINYUSB_NO_NETUTILS 1
#endif

#ifndef TINYUSB_IOCTL_TIMEOUT_US
#define TINYUSB_IOCTL_TIMEOUT_US 1000000
#endif

#ifndef TINYUSB_USE_STATS
#define TINYUSB_USE_STATS 0
#endif

// todo should this be user settable?
#ifndef TINYUSB_HAL_MAC_EX0
#define TINYUSB_HAL_MAC_EX0 0
#endif

#ifndef STATIC
#define STATIC static
#endif

#endif