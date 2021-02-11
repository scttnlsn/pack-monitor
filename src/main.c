#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "config.h"
#include "led.h"
#include "modbus.h"
#include "onewire.h"
#include "ds18b20.h"
#include "cell_monitors.h"

bool timer_callback(repeating_timer_t *rt);

modbus_t modbus;
uint16_t registers[1024];

onewire_t onewire = {
  .pin = ONEWIRE_PIN,
};

cell_monitors_t cell_monitors;

queue_t timer_fifo;

int main() {
  stdio_init_all();

  queue_init(&timer_fifo, sizeof(uint8_t), 32);

  repeating_timer_t timer;
  add_repeating_timer_ms(3000, timer_callback, NULL, &timer);

  led_init();

  modbus_init(&modbus);
  modbus.unit_address = 1;
  modbus.registers = registers;
  modbus.num_registers = sizeof(registers);

  onewire_init(&onewire);

  cell_monitors_init(&cell_monitors);

  led_blink();

  sleep_ms(1000);
  if (!cell_monitors_connect(&cell_monitors)) {
    puts("err: could not connect to cell monitors");
  }

  while (1) {
    uint8_t queue_element;
    while (queue_try_remove(&timer_fifo, &queue_element)) {
      // timer expired
      uint32_t temp = ds18b20_read_temp(&onewire);
      registers[0] = (temp >> 16) & 0xFFFF;
      registers[1] = temp & 0xFFFF;

      uint16_t voltage = 0;
      cell_monitors_read_voltage(&cell_monitors, 1, &voltage);
      registers[2] = voltage;
      cell_monitors_read_voltage(&cell_monitors, 2, &voltage);
      registers[3] = voltage;
    }

    modbus_update(&modbus);
    sleep_ms(1);

    led_blink();
  }
}

bool timer_callback(repeating_timer_t *rt) {
  uint8_t queue_element;
  queue_try_add(&timer_fifo, &queue_element);

  // keep repeating
  return true;
}
