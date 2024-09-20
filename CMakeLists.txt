cmake_minimum_required(VERSION 3.13)

include(pico-sdk/external/pico_sdk_import.cmake)

project(pack_monitor_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(
  pack_monitor
  src/main.c
  src/events.c
  src/led.c
  src/ringbuf.c
  src/modbus.c
  src/onewire.c
  src/ds18b20.c
  src/cell_monitors.c
)

pico_enable_stdio_usb(pack_monitor 1)
pico_add_extra_outputs(pack_monitor)

target_link_libraries(
  pack_monitor
  pico_stdlib
  hardware_pwm
)
