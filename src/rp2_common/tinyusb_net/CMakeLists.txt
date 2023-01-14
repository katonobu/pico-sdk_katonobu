if (DEFINED ENV{PICO_TINYUSB_PATH} AND (NOT PICO_TINYUSB_PATH))
    set(PICO_TINYUSB_PATH $ENV{PICO_TINYUSB_PATH})
    message("Using PICO_TINYUSB_PATH from environment ('${PICO_TINYUSB_PATH}')")
endif ()

set(TINYUSB_NET_TEST_PATH "examples/device/net_lwip_webserver/src")
if (NOT PICO_TINYUSB_PATH)
    set(PICO_TINYUSB_PATH ${PROJECT_SOURCE_DIR}/lib/tinyusb)
    if (NOT EXISTS ${PICO_TINYUSB_PATH}/${TINYUSB_NET_TEST_PATH})
        message(WARNING "TinyUSB submodule has not been initialized; USB support will be unavailable
hint: try 'git submodule update --init' from your SDK directory (${PICO_SDK_PATH}).")
    endif()
elseif (NOT EXISTS ${PICO_TINYUSB_PATH}/${TINYUSB_NET_TEST_PATH})
    message(WARNING "PICO_TINYUSB_PATH specified but content not present.")
endif()

if (EXISTS ${PICO_TINYUSB_PATH}/${TINYUSB_NET_TEST_PATH})
    add_library(tinyusb_driver_pico INTERFACE)

    target_sources(tinyusb_driver_pico INTERFACE
        ${PICO_TINYUSB_PATH}/${TINYUSB_NET_TEST_PATH}/usb_descriptors.c
        )
    target_link_libraries(tinyusb_driver_pico INTERFACE
        tinyusb_device
        tinyusb_board
        pico_stdlib
        )

    pico_promote_common_scope_vars()
endif()
