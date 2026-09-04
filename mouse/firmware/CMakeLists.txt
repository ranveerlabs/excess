cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(excess_mouse C CXX ASM)
pico_sdk_init()

add_executable(mouse main.c pmw3360.c usb_descriptors.c)
target_include_directories(mouse PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(mouse pico_stdlib hardware_spi tinyusb_device tinyusb_board)

pico_enable_stdio_usb(mouse 0)
pico_enable_stdio_uart(mouse 1)
pico_add_extra_outputs(mouse)
