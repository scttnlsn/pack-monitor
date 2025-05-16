cmake_minimum_required(VERSION 3.13)

include(pico-sdk/external/pico_sdk_import.cmake)

project(pack_monitor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(
  ${PROJECT_NAME}
  src/main.c
  src/events.c
  src/led.c
  src/ringbuf.c
  src/modbus.c
  src/onewire.c
  src/temp.c
  src/cell_monitors.c
  src/protection.c
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(
  ${PROJECT_NAME}
  pico_stdlib
  hardware_pwm
  hardware_exception
)
