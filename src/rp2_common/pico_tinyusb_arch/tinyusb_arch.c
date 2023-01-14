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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/unique_id.h"
#include "tinyusb.h"
#include "pico/tinyusb_arch.h"
#include "tinyusb_ll.h"     // ToDo:check available or alternate
#include "tinyusb_stats.h"  // ToDo:check available or alternate

#if TINYUSB_ARCH_DEBUG_ENABLED
#define TINYUSB_ARCH_DEBUG(...) printf(__VA_ARGS__)
#else
#define TINYUSB_ARCH_DEBUG(...) ((void)0)
#endif

#if TINYUSB_ARCH_DEBUG_ENABLED
// Return a string for the wireless state
/*
static const char* status_name(int status)
{
    // ToDo:must be adjust to USB connection
    switch (status) {
    case TINYUSB_LINK_DOWN:
        return "link down";
    case TINYUSB_LINK_JOIN:
        return "joining";
    case TINYUSB_LINK_NOIP:
        return "no ip";
    case TINYUSB_LINK_UP:
        return "link up";
    case TINYUSB_LINK_FAIL:
        return "link fail";
    case TINYUSB_LINK_NONET:
        return "network fail";
    case TINYUSB_LINK_BADAUTH:
        return "bad auth";
    }
    return "unknown";
}
*/
#endif

int tinyusb_arch_connect_async(void) {
    // Connect to wireless
    // return tinyusb_join(&tinyusb_state); // ToDo:adjust to USB
    return 0;
}

// Connect to wireless, return with success when an IP address has been assigned
int tinyusb_arch_wifi_connect_until(const char *ssid, const char *pw, uint32_t auth, absolute_time_t until) {
    int err = tinyusb_arch_connect_async();
    if (err) return err;

    int status = TINYUSB_LINK_UP + 1;
    while(status >= 0 && status != TINYUSB_LINK_UP) {
        /* ToDo:adjust to USB
        int new_status = tinyusb_tcpip_link_status(&tinyusb_state, TINYUSB_ITF_STA);
        if (new_status != status) {
            status = new_status;
            TINYUSB_ARCH_DEBUG("connect status: %s\n", status_name(status));
        }
        */
        // in case polling is required
        tinyusb_arch_poll();
        best_effort_wfe_or_timeout(until);
        if (time_reached(until)) {
            return PICO_ERROR_TIMEOUT;
        }
    }
    return status == TINYUSB_LINK_UP ? 0 : status;
}

int tinyusb_arch_connect_blocking(void ) {
    return tinyusb_arch_connect_until(ssid, pw, auth, at_the_end_of_time);
}

int tinyusb_arch_connect_timeout_ms(uint32_t timeout_ms) {
    return tinyusb_arch_connect_until(make_timeout_time_ms(timeout_ms));
}

// todo maybe add an #ifdef in tinyusb_driver
uint32_t storage_read_blocks(__unused uint8_t *dest, __unused uint32_t block_num, __unused uint32_t num_blocks) {
    // shouldn't be used
    panic_unsupported();
}

// Generate a mac address if one is not set in otp
void tinyusb_hal_generate_laa_mac(__unused int idx, uint8_t buf[6]) {
    TINYUSB_DEBUG("Warning. No mac in otp. Generating mac from board id\n");
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    memcpy(buf, &board_id.id[2], 6);
    buf[0] &= (uint8_t)~0x1; // unicast
    buf[0] |= 0x2; // locally administered
}

// Return mac address
void tinyusb_hal_get_mac(__unused int idx, uint8_t buf[6]) {
    // The mac should come from USB Serial number.
    // This is loaded into the state after the driver is initialised
    // tinyusb_hal_generate_laa_mac is called by the driver to generate a mac if
    memcpy(buf, tinyusb_state.mac, 6);
}

