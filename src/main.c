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
repeating_timer_t timer;

bool timer_callback(repeating_timer_t *rt) {
  uint8_t queue_element;
  queue_try_add(&timer_fifo, &queue_element);

  // keep repeating
  return true;
}

void start_timer() {
  add_repeating_timer_ms(5000, timer_callback, NULL, &timer);
}

void stop_timer() {
  cancel_repeating_timer(&timer);
}

void connect_loop() {
  stop_timer();
  led_pulse_start();

  while (1) {
    puts("connecting to cell monitors...");
    if (cell_monitors_connect(&cell_monitors)) {
      puts("connected");
      led_pulse_stop();
      start_timer();
      return;
    }

    // continue trying to reconnect every 3s
    sleep_ms(3000);
  }
}

int main() {
  stdio_init_all();

  queue_init(&timer_fifo, sizeof(uint8_t), 32);

  led_init();

  modbus_init(&modbus);
  modbus.unit_address = 1;
  modbus.registers = registers;
  modbus.num_registers = sizeof(registers);

  onewire_init(&onewire);

  cell_monitors_init(&cell_monitors);

  led_blink();

  sleep_ms(1000);
  connect_loop();

  while (1) {
    if (!cell_monitors.connected) {
      connect_loop();
    }
    
    uint8_t queue_element;
    while (queue_try_remove(&timer_fifo, &queue_element)) {
      // timer expired

      // 2 blinks to signal the start of a measurement
      led_blink();
      led_blink();

      uint32_t temp = ds18b20_read_temp(&onewire);
      registers[0] = (temp >> 16) & 0xFFFF;
      registers[1] = temp & 0xFFFF;

      for (uint16_t cell = 1; cell <= cell_monitors.num_cells; cell++) {
        uint16_t voltage = 0;
        cell_monitors_read_voltage(&cell_monitors, cell, &voltage);
        registers[cell + 1] = voltage;
      }

      // 1 blink to signal the end of a measurement
      led_blink();
    }

    modbus_update(&modbus);
    sleep_ms(1);
  }
}
