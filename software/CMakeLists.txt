cmake_minimum_required(VERSION 3.24)
project(bus_test_platform_sw VERSION 0.1.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(BT_BUILD_TESTS    "Build unit tests (minitest, no external deps)" ON)
option(BT_ENABLE_UI      "Build Qt6 UI layer (requires Qt6 Widgets)"       OFF)
option(BT_ENABLE_PYTHON  "Build Python DSL host (requires Python3 dev)"    OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ---------------------------------------------------------------------------
# Shared single-source-of-truth headers (usb_protocol.h / bus_types.h)
# ---------------------------------------------------------------------------
add_library(bt_shared INTERFACE)
target_include_directories(bt_shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)
target_compile_features(bt_shared INTERFACE cxx_std_17)

# ---------------------------------------------------------------------------
# L0 core: object model (BusFrame helpers, Channel, Signal, Node, Workspace)
# ---------------------------------------------------------------------------
add_library(bt_core STATIC
    src/core/bus_frame.cpp
    src/core/channel.cpp
    src/core/signal.cpp
    src/core/node.cpp
    src/core/workspace.cpp
)
target_link_libraries(bt_core PUBLIC bt_shared)
target_include_directories(bt_core PUBLIC src)

# ---------------------------------------------------------------------------
# L3 protocol stack: CAN decode, DBC parser, USB frame codec
# ---------------------------------------------------------------------------
add_library(bt_protocol STATIC
    src/protocol/can/can_decoder.cpp
    src/protocol/dbc/dbc_parser.cpp
    src/protocol/usb/usb_frame_codec.cpp
)
target_link_libraries(bt_protocol PUBLIC bt_core)

# ---------------------------------------------------------------------------
# L2 data: zero-copy ring pipeline
# ---------------------------------------------------------------------------
add_library(bt_data STATIC
    src/data/pipeline.cpp
)
target_link_libraries(bt_data PUBLIC bt_core)

# ---------------------------------------------------------------------------
# L4 engines: RBS scheduler (+ interface-only script host / test runner / diag)
# ---------------------------------------------------------------------------
add_library(bt_engine STATIC
    src/engine/rbs/rbs_scheduler.cpp
)
target_link_libraries(bt_engine PUBLIC bt_core bt_protocol)

# ---------------------------------------------------------------------------
# L1 HAL: USB device abstraction + null device for self-test
# ---------------------------------------------------------------------------
add_library(bt_hal STATIC
    src/hal/usb/usb_device_null.cpp
)
target_link_libraries(bt_hal PUBLIC bt_core)

# ---------------------------------------------------------------------------
# CLI: busmon
# ---------------------------------------------------------------------------
add_executable(busmon src/main.cpp)
target_link_libraries(busmon PRIVATE bt_hal bt_engine bt_data bt_protocol bt_core)
target_compile_definitions(busmon PRIVATE BT_PROJECT_VERSION="${PROJECT_VERSION}")

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------
if(BT_BUILD_TESTS)
    enable_testing()
    add_executable(bt_unit_tests
        tests/unit/test_main.cpp
        tests/unit/test_bus_frame.cpp
        tests/unit/test_dbc_parser.cpp
        tests/unit/test_usb_codec.cpp
        tests/unit/test_rbs_scheduler.cpp
        tests/unit/test_pipeline.cpp
    )
    target_link_libraries(bt_unit_tests PRIVATE bt_hal bt_engine bt_data bt_protocol bt_core)
    target_include_directories(bt_unit_tests PRIVATE tests/unit)
    add_test(NAME bt_unit_tests COMMAND bt_unit_tests)
endif()

# ---------------------------------------------------------------------------
# Optional Qt6 UI
# ---------------------------------------------------------------------------
if(BT_ENABLE_UI)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    add_library(bt_ui STATIC
        src/ui/mainwindow.cpp
    )
    target_link_libraries(bt_ui PUBLIC bt_core Qt6::Widgets)
    target_include_directories(bt_ui PUBLIC src)
endif()
