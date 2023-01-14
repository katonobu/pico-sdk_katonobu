if (TARGET tinyusb_driver_pico)
    message("Enabling build support for USB-ether.")

    pico_add_impl_library(pico_tinyusb_arch)
    target_sources(pico_tinyusb_arch INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/tinyusb_arch.c
            ${CMAKE_CURRENT_LIST_DIR}/tinyusb_arch_poll.c
            ${CMAKE_CURRENT_LIST_DIR}/tinyusb_arch_threadsafe_background.c
            ${CMAKE_CURRENT_LIST_DIR}/tinyusb_arch_freertos.c
            )

    target_include_directories(pico_tinyusb_arch INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/include)

    target_link_libraries(pico_tinyusb_arch INTERFACE
            pico_unique_id
            tinyusb_driver_pico)

    if (NOT TARGET pico_lwip)
        message(WARNING "lwIP is not available; USB-ether support will be unavailable too")
    else()
        add_library(pico_tinyusb_arch_lwip_poll INTERFACE)
        target_link_libraries(pico_tinyusb_arch_lwip_poll INTERFACE
                pico_tinyusb_arch
                pico_lwip
                pico_lwip_nosys)
        target_compile_definitions(pico_tinyusb_arch_lwip_poll INTERFACE
                TINYUSB_LWIP=1
                PICO_TINYUSB_ARCH_POLL=1
                )

        add_library(pico_tinyusb_arch_lwip_threadsafe_background INTERFACE)
        target_link_libraries(pico_tinyusb_arch_lwip_threadsafe_background INTERFACE
                pico_tinyusb_arch
                pico_lwip
                pico_lwip_nosys)
        target_compile_definitions(pico_tinyusb_arch_lwip_threadsafe_background INTERFACE
                TINYUSB_LWIP=1
                PICO_TINYUSB_ARCH_THREADSAFE_BACKGROUND=1
                )

        add_library(pico_tinyusb_arch_lwip_sys_freertos INTERFACE)
        target_link_libraries(pico_tinyusb_arch_lwip_sys_freertos INTERFACE
                pico_tinyusb_arch
                pico_lwip
                pico_lwip_contrib_freertos)
        target_compile_definitions(pico_tinyusb_arch_lwip_sys_freertos INTERFACE
                TINYUSB_LWIP=1
                LWIP_PROVIDE_ERRNO=1
                PICO_TINYUSB_ARCH_FREERTOS=1
                )
    endif()
endif()

if (PICO_TINYUSB_DRIVER_PATH AND EXISTS "${PICO_TINYUSB_DRIVER_PATH}")
    pico_add_doxygen(${PICO_TINYUSB_DRIVER_PATH}/src)
endif()