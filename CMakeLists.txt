cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pack_monitor_project)

pico_sdk_init()

add_executable(pack_monitor main.c)

pico_enable_stdio_usb(pack_monitor 1)
pico_add_extra_outputs(pack_monitor)

target_link_libraries(pack_monitor pico_stdlib)
