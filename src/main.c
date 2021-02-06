#include <stdio.h>
#include "pico/stdlib.h"

#include "led.h"
#include "modbus.h"

static modbus_t modbus;
static uint16_t registers[1024];

int main() {
  stdio_init_all();

  led_init();
  modbus_init(&modbus);
  modbus.unit_address = 1;
  modbus.registers = registers;
  modbus.num_registers = sizeof(registers);

  led_blink();

  while (1) {
    modbus_update(&modbus);
  }
}
