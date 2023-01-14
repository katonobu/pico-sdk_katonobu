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

#ifndef _PICO_TINYUSB_ARCH_H
#define _PICO_TINYUSB_ARCH_H

#include "pico.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "tinyusb.h"

/**
 * \defgroup tinyusb_driver tinyusb_driver
 * \ingroup pico_tinyusb_arch
 * \brief Driver used for USB-net
*/

/**
 * \defgroup tinyusb_ll tinyusb_ll
 * \ingroup tinyusb_driver
 * \brief Low Level TINYUSB driver interface
*/

/** \file pico/tinyusb_arch.h
 *  \defgroup pico_tinyusb_arch pico_tinyusb_arch
 *
 * Architecture for integrating the TINYUSB driver (for the ether on USB) and lwIP (for TCP/IP stack) into the SDK.
 *
 * Both the low level \c tinyusb_driver and the lwIP stack require periodic servicing, and have limitations
 * on whether they can be called from multiple cores/threads.
 *
 * \c pico_tinyusb_arch attempts to abstract these complications into several behavioral groups:
 *
 * * \em 'poll' - This not multi-core/IRQ safe, and requires the user to call \ref tinyusb_arch_poll periodically from their main loop
 * * \em 'thread_safe_background' - This is multi-core/thread/task safe, and maintenance of the driver and TCP/IP stack is handled automatically in the background
 *
 * As of right now, lwIP is the only supported TCP/IP stack, however the use of \c pico_tinyusb_arch is intended to be independent of
 * the particular TCP/IP stack used (and possibly Bluetooth stack used) in the future. For this reason, the integration of lwIP
 * is handled in the base (\c pico_tinyusb_arch) library based on the #define \ref TINYUSB_LWIP used by the \c tinyusb_driver.
 *
 * Whilst you can use the \c pico_tinyusb_arch library directly and  specify \ref TINYUSB$#_LWIP (and other defines) yourself, several
 * other libraries are made available to the build which aggregate the defines and other dependencies for you:
 *
 * * \b pico_tinyusb_arch_lwip_poll - For using the RAW lwIP API (in `NO_SYS=1` mode) without any background processing or multi-core/thread safety.
 *
 *    The user must call \ref pico_tinyusb_poll periodically from their main loop.
 *
 *    This wrapper library:
 *    - Sets \c TINYUSB_LWIP=1 to enable lwIP support in \c pico_tinyusb_arch and \c tinyusb_driver.
 *    - Sets \c PICO_TINYUSB_ARCH_POLL=1 to select the polling behavior.
 *    - Adds the \c pico_lwip as a dependency to pull in lwIP.
 *
 * * \b pico_tinyusb_arch_lwip_threadsafe_background - For using the RAW lwIP API (in `NO_SYS=1` mode) with multi-core/thread safety, and automatic servicing of the \c tinyusb_driver and
 * lwIP in background.
 *
 *    Calls into the \c tinyusb_driver high level API (tinyusb.h) may be made from either core or from lwIP callbacks, however calls into lwIP (which
 * is not thread-safe) other than those made from lwIP callbacks, must be bracketed with \ref tinyusb_arch_lwip_begin and \ref tinyusb_arch_lwip_end. It is fine to bracket
 * calls made from within lwIP callbacks too; you just don't have to.
 *
 *    \note lwIP callbacks happen in a (low priority) IRQ context (similar to an alarm callback), so care should be taken when interacting
 *    with other code.
 *
 *    This wrapper library:
 *    - Sets \c TINYUSB_LWIP=1 to enable lwIP support in \c pico_tinyusb_arch and \c tinyusb_driver
 *    - Sets \c PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND=1 to select the thread-safe/non-polling behavior.
 *    - Adds the pico_lwip as a dependency to pull in lwIP.
 *
 *
 *    This library \em can also be used under the RP2040 port of FreeRTOS with lwIP in `NO_SYS=1` mode (allowing you to call \c tinyusb_driver APIs
 * from any task, and to call lwIP from lwIP callbacks, or from any task if you bracket the calls with \ref tinyusb_arch_lwip_begin and \ref tinyusb_arch_lwip_end. Again, you should be
 * careful about what you do in lwIP callbacks, as you cannot call most FreeRTOS APIs from within an IRQ context. Unless you have good reason, you should probably
 * use the full FreeRTOS integration (with `NO_SYS=0`) provided by \c pico_tinyusb_arch_lwip_sys_freertos.
 *
 * * \b pico_tinyusb_arch_lwip_sys_freertos - For using the full lwIP API including blocking sockets in OS (`NO_SYS=0`) mode, along with with multi-core/task/thread safety, and automatic servicing of the \c tinyusb_driver and
 * the lwIP stack.
 *
 *    This wrapper library:
 *    - Sets \c TINYUSB_LWIP=1 to enable lwIP support in \c pico_tinyusb_arch and \c tinyusb_driver.
 *    - Sets \c PICO_TINYUSB_ARCH_FREERTOS=1 to select the NO_SYS=0 lwip/FreeRTOS integration
 *    - Sets \c LWIP_PROVIDE_ERRNO=1 to provide error numbers needed for compilation without an OS
 *    - Adds the \c pico_lwip as a dependency to pull in lwIP.
 *    - Adds the lwIP/FreeRTOS code from lwip-contrib (in the contrib directory of lwIP)
 *
 *    Calls into the \c tinyusb_driver high level API (tinyusb.h) may be made from any task or from lwIP callbacks, but not from IRQs. Calls into the lwIP RAW API (which is not thread safe)
 *    must be bracketed with \ref tinyusb_arch_lwip_begin and \ref tinyusb_arch_lwip_end. It is fine to bracket calls made from within lwIP callbacks too; you just don't have to.
 *
 *    \note this wrapper library requires you to link FreeRTOS functionality with your application yourself.
 *
 * * \b pico_tinyusb_arch_none - If you do not need the TCP/IP stack but wish to use the on-board LED.
 *
 *    This wrapper library:
 *    - Sets \c TINYUSB_LWIP=0 to disable lwIP support in \c pico_tinyusb_arch and \c tinyusb_driver
 */

// PICO_CONFIG: PARAM_ASSERTIONS_ENABLED_TINYUSB_ARCH, Enable/disable assertions in the pico_tinyusb_arch module, type=bool, default=0, group=pico_tinyusb_arch
#ifndef PARAM_ASSERTIONS_ENABLED_TINYUSB_ARCH
#define PARAM_ASSERTIONS_ENABLED_TINYUSB_ARCH 0
#endif

// PICO_CONFIG: TINYUSB_ARCH_DEBUG_ENABLED, Enable/disable some debugging output in the pico_tinyusb_arch module, type=bool, default=1 in debug builds, group=pico_tinyusb_arch
#ifndef TINYUSB_ARCH_DEBUG_ENABLED
#ifndef NDEBUG
#define TINYUSB_ARCH_DEBUG_ENABLED 1
#else
#define TINYUSB_ARCH_DEBUG_ENABLED 0
#endif
#endif

// PICO_CONFIG: PICO_TINYUSB_ARCH_DEFAULT_COUNTRY_CODE, Default country code for the tinyusb wireless driver, default=TINYUSB_COUNTRY_WORLDWIDE, group=pico_tinyusb_arch
#ifndef PICO_TINYUSB_ARCH_DEFAULT_COUNTRY_CODE
#define PICO_TINYUSB_ARCH_DEFAULT_COUNTRY_CODE TINYUSB_COUNTRY_WORLDWIDE
#endif

/*!
 * \brief Initialize the TINYUSB architecture
 * \ingroup pico_tinyusb_arch
 *
 * This method initializes the `tinyusb_driver` code and initializes the lwIP stack (if it
 * was enabled at build time). This method must be called prior to using any other \c pico_tinyusb_arch,
 * \tinyusb_driver or lwIP functions.
 *
 * \return 0 if the initialization is successful, an error code otherwise \see pico_error_codes
 */
int tinyusb_arch_init(void);

/*!
 * \brief De-initialize the TINYUSB architecture
 * \ingroup pico_tinyusb_arch
 *
 * This method de-initializes the `tinyusb_driver` code and de-initializes the lwIP stack (if it
 * was enabled at build time). Note this method should always be called from the same core (or RTOS
 * task, depending on the environment) as \ref tinyusb_arch_init.
 */
void tinyusb_arch_deinit(void);

/*!
 * \brief Attempt to connect to USB, blocking until the network is joined or a failure is detected.
 * \ingroup pico_tinyusb_arch
 *
 * \return 0 if the initialization is successful, an error code otherwise \see pico_error_codes
 */
int tinyusb_arch_connect_blocking(void);

/*!
 * \brief Attempt to connect to USB, blocking until the network is joined, a failure is detected or a timeout occurs
 * \ingroup pico_tinyusb_arch
 *
 * \param timeout the max timeout to connect.
 *
 * \return 0 if the initialization is successful, an error code otherwise \see pico_error_codes
 */
int tinyusb_arch_connect_timeout_ms(uint32_t timeout);

/*!
 * \brief Start attempting to connect to USB
 * \ingroup pico_tinyusb_arch
 *
 * This method tells the TINYUSB driver to start connecting to USB. You should subsequently check the
 * status by calling \ref tinyusb_usb_link_status.
 *
 * \return 0 if the scan was started successfully, an error code otherwise \see pico_error_codes
 */
int tinyusb_arch_connect_async(void);

/*!
 * \brief Perform any processing required by the \c tinyusb_driver or the TCP/IP stack
 * \ingroup pico_tinyusb_arch
 *
 * This method must be called periodically from the main loop when using a
 * \em polling style \c pico_tinyusb_arch (e.g. \c pico_tinyusb_arch_lwip_poll ). It
 * may be called in other styles, but it is unnecessary to do so.
 */
void tinyusb_arch_poll(void);

/*!
 * \fn tinyusb_arch_lwip_begin
 * \brief Acquire any locks required to call into lwIP
 * \ingroup pico_tinyusb_arch
 *
 * The lwIP API is not thread safe. You should surround calls into the lwIP API
 * with calls to this method and \ref tinyusb_arch_lwip_end. Note these calls are not
 * necessary (but harmless) when you are calling back into the lwIP API from an lwIP callback.
 * If you are using single-core polling only (pico_tinyusb_arch_poll) then these calls are no-ops
 * anyway it is good practice to call them anyway where they are necessary.
 *
 * \sa tinyusb_arch_lwip_end
 * \sa tinyusb_arch_lwip_protect
 */

/*!
 * \fn void tinyusb_arch_lwip_end(void)
 * \brief Release any locks required for calling into lwIP
 * \ingroup pico_tinyusb_arch
 *
 * The lwIP API is not thread safe. You should surround calls into the lwIP API
 * with calls to \ref tinyusb_arch_lwip_begin and this method. Note these calls are not
 * necessary (but harmless) when you are calling back into the lwIP API from an lwIP callback.
 * If you are using single-core polling only (pico_tinyusb_arch_poll) then these calls are no-ops
 * anyway it is good practice to call them anyway where they are necessary.
 *
 * \sa tinyusb_arch_lwip_begin
 * \sa tinyusb_arch_lwip_protect
 */

/*!
 * \fn int tinyusb_arch_lwip_protect(int (*func)(void *param), void *param)
 * \brief sad Release any locks required for calling into lwIP
 * \ingroup pico_tinyusb_arch
 *
 * The lwIP API is not thread safe. You can use this method to wrap a function
 * with any locking required to call into the lwIP API. If you are using
 * single-core polling only (pico_tinyusb_arch_poll) then there are no
 * locks to required, but it is still good practice to use this function.
 *
 * \param func the function ta call with any required locks held
 * \param param parameter to pass to \c func
 * \return the return value from \c func
 * \sa tinyusb_arch_lwip_begin
 * \sa tinyusb_arch_lwip_end
 */

/*!
 * \fn void tinyusb_arch_lwip_check(void)
 * \brief Checks the caller has any locks required for calling into lwIP
 * \ingroup pico_tinyusb_arch
 *
 * The lwIP API is not thread safe. You should surround calls into the lwIP API
 * with calls to \ref tinyusb_arch_lwip_begin and this method. Note these calls are not
 * necessary (but harmless) when you are calling back into the lwIP API from an lwIP callback.
 *
 * This method will assert in debug mode, if the above conditions are not met (i.e. it is not safe to
 * call into the lwIP API)
 *
 * \sa tinyusb_arch_lwip_begin
 * \sa tinyusb_arch_lwip_protect
 */

#ifdef __cplusplus
}
#endif

#endif
