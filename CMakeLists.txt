cmake_minimum_required(VERSION 3.17)
include(pico_sdk_import.cmake)
set(ENV{PICO_SDK_PATH} D:/Projects/pico/pico-sdk/)

project(pico_ledstrip C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(pico_ledstrip main.cpp)
pico_sdk_init()

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(pico_ledstrip ${CMAKE_CURRENT_LIST_DIR}/ws2811.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(pico_ledstrip pico_stdlib hardware_pio hardware_uart)

# enable usb output, disable uart output
pico_enable_stdio_usb(pico_ledstrip 1)
pico_enable_stdio_uart(pico_ledstrip 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(pico_ledstrip)
