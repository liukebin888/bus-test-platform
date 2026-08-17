cmake_minimum_required(VERSION 3.24)
project(bt_firmware VERSION 0.1.0 LANGUAGES C)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(BT_FW_TARGET_STM32 "Build firmware for STM32H750 (needs ARM toolchain)" OFF)
option(BT_FW_BUILD_HOST_TESTS "Build host-side protocol unit tests" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Shared single-source-of-truth headers (usb_protocol.h / bus_types.h)
set(BT_SHARED_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)

# Protocol layer is pure logic, compiled for both targets.
set(BT_PROTOCOL_SOURCES
    src/protocol/usb_protocol_handler.c
)

if(BT_FW_TARGET_STM32)
    # -----------------------------------------------------------------------
    # Real STM32H750 firmware image
    # -----------------------------------------------------------------------
    if(NOT CMAKE_TOOLCHAIN_FILE)
        message(FATAL_ERROR
            "BT_FW_TARGET_STM32=ON requires -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake")
    endif()

    set(BT_FW_SOURCES
        src/main.c
        src/app/app_config.h
        src/drivers/fdcan/fdcan.c
        src/drivers/lin/lin.c
        src/drivers/usb/usb_device.c
        src/drivers/dfu/dfu.c
        ${BT_PROTOCOL_SOURCES}
    )

    add_executable(bt_fw.elf ${BT_FW_SOURCES})

    target_include_directories(bt_fw.elf PRIVATE
        config
        src
        src/app
        src/drivers
        ${BT_SHARED_INCLUDE}
    )

    target_compile_options(bt_fw.elf PRIVATE
        -mcpu=cortex-m7 -mthumb -mfloat-abi=soft -O2 -Wall -Wextra
    )
    target_link_options(bt_fw.elf PRIVATE
        -mcpu=cortex-m7 -mthumb -mfloat-abi=soft
        -T${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32h750_flash.ld
        -nostartfiles
    )

    # Post-build: print sizes and produce .bin/.hex artifacts
    find_program(ARM_SIZE arm-none-eabi-size)
    find_program(ARM_OBJCOPY arm-none-eabi-objcopy)
    if(ARM_OBJCOPY)
        add_custom_command(TARGET bt_fw.elf POST_BUILD
            COMMAND ${ARM_OBJCOPY} -O binary $<TARGET_FILE:bt_fw.elf>
                      ${CMAKE_CURRENT_BINARY_DIR}/bt_fw.bin
            COMMAND ${ARM_OBJCOPY} -O ihex $<TARGET_FILE:bt_fw.elf>
                      ${CMAKE_CURRENT_BINARY_DIR}/bt_fw.hex
            COMMENT "Generate bt_fw.bin / bt_fw.hex")
    endif()
    if(ARM_SIZE)
        add_custom_command(TARGET bt_fw.elf POST_BUILD
            COMMAND ${ARM_SIZE} $<TARGET_FILE:bt_fw.elf>
            COMMENT "Firmware size:")
    endif()
else()
    # -----------------------------------------------------------------------
    # Host-side protocol unit tests (no hardware toolchain required)
    # -----------------------------------------------------------------------
    if(BT_FW_BUILD_HOST_TESTS)
        add_executable(fw_protocol_test
            tests/host/test_usb_protocol.c
            ${BT_PROTOCOL_SOURCES}
        )
        target_include_directories(fw_protocol_test PRIVATE
            src
            src/protocol
            ${BT_SHARED_INCLUDE}
        )
        target_compile_options(fw_protocol_test PRIVATE -Wall -Wextra)
        enable_testing()
        add_test(NAME fw_protocol_test COMMAND fw_protocol_test)
    endif()
endif()
