#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "config.h"
#include "events.h"
#include "led.h"
#include "modbus.h"
#include "onewire.h"
#include "ds18b20.h"
#include "cell_monitors.h"

#define EVENT_TYPE_MEASURE 0

modbus_t modbus;
uint16_t registers[1024];

onewire_t onewire = {
  .pin = ONEWIRE_PIN,
};

cell_monitors_t cell_monitors;

repeating_timer_t timer;

bool timer_callback(repeating_timer_t *rt) {
  event_t event = {
    .event_type = EVENT_TYPE_MEASURE,
    .data = NULL,
  };
  events_enqueue(event);

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

void collect_measurements() {
  // 2 blinks to signal the start of a measurement
  led_blink();
  led_blink();

  uint16_t reg = 0;

  uint32_t temp = ds18b20_read_temp(&onewire);
  registers[reg++] = (temp >> 16) & 0xFFFF;
  registers[reg++] = temp & 0xFFFF;

  for (uint16_t i = 0; i < cell_monitors.num_cells; i++) {
    uint8_t cell_address = i + 1;
    uint16_t voltage = 0;
    cell_monitors_read_voltage(&cell_monitors, cell_address, &voltage);

    registers[reg++] = voltage;
  }

  // 1 blink to signal the end of a measurement
  led_blink();
}

int main() {
  stdio_init_all();

  events_init();
  led_init();

  modbus_init(&modbus);
  modbus.unit_address = 1;
  modbus.registers = registers;
  modbus.num_registers = sizeof(registers);

  onewire_init(&onewire);

  cell_monitors_init(&cell_monitors);
  connect_loop();

  while (1) {
    if (!cell_monitors.connected) {
      connect_loop();
    }
    
    event_t event;
    while (events_dequeue(&event)) {
      if (event.event_type == EVENT_TYPE_MEASURE) {
        collect_measurements();
      }
    }

    modbus_update(&modbus);
    sleep_ms(1);
  }
}
