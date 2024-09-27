#include <stdio.h>
#include <stddef.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/exception.h"

#include "config.h"
#include "events.h"
#include "led.h"
#include "modbus.h"
#include "onewire.h"
#include "ds18b20.h"
#include "cell_monitors.h"

// packed and aligned so we can treat it as an array of uint16_t registers
typedef struct __attribute__((packed, aligned(sizeof(uint32_t)))) {
  uint16_t version;
  uint16_t connected;
  uint16_t error_code;
  uint16_t num_cells;
  union {
    uint32_t temp;
    struct {
      uint16_t temp_upper;
      uint16_t temp_lower;
    };
  };

  uint16_t cell_voltages[MAX_CELLS];
  uint16_t cell_voltage_refs[MAX_CELLS];
} registers_t;

registers_t registers;
onewire_t onewire = { .pin = ONEWIRE_PIN };
modbus_t modbus;
cell_monitors_t cell_monitors;
repeating_timer_t measurements_timer;

bool measurements_timer_callback(repeating_timer_t *rt) {
  event_t event = {
    .event_type = EVENT_TYPE_MEASURE,
  };
  events_enqueue(event);

  // keep repeating
  return true;
}

static void modbus_write_callback(uint16_t reg, uint16_t previous_value, uint16_t current_value) {
  event_t event = {
    .event_type = EVENT_TYPE_REGISTER,
    .reg_info = {
      .reg = reg,
      .previous_value = previous_value,
      .current_value = current_value,
    },
  };
  events_enqueue(event);
}

void connect_loop() {
  cancel_repeating_timer(&measurements_timer);
  led_pulse_start();

  while (1) {
    puts("connecting to cell monitors...");
    if (cell_monitors_connect(&cell_monitors)) {
      puts("connected");
      led_pulse_stop();

      if (registers.num_cells == 0) {
        registers.num_cells = cell_monitors.num_cells;
      }
      add_repeating_timer_ms(MEASUREMENTS_INTERAL_MS, measurements_timer_callback, NULL, &measurements_timer);
      return;
    }

    // continue trying to reconnect
    sleep_ms(CONNECT_DELAY_MS);
  }
}

void collect_measurements() {
  // 2 blinks to signal the start of a measurement
  led_blink();
  led_blink();

  registers.temp = ds18b20_read_temp(&onewire);

  for (uint16_t i = 0; i < cell_monitors.num_cells; i++) {
    uint8_t cell_address = i + 1;
    uint16_t voltage = 0;
    cell_monitors_read(&cell_monitors, cell_address, CELL_MONITORS_REG_VOLTAGE, &voltage);
    registers.cell_voltages[i] = voltage;
  }

  // 1 blink to signal the end of a measurement
  led_blink();
}

void handle_register_written(uint16_t reg, uint16_t previous, uint16_t current) {
  // handle cases where the write to a register means we need to
  // write to a cell monitor

  // we want uint16_t offset but offsetof returns bytes
  uint16_t offset = offsetof(registers_t, cell_voltage_refs) / 2;

  if (offset <= reg && reg < offset + MAX_CELLS) {
    // a cell voltage reference was updated
    uint16_t cell_offset = reg - offset;
    uint16_t cell_address = cell_offset + 1;
    uint16_t voltage_ref = registers.cell_voltage_refs[cell_offset];
    if (!cell_monitors_write(&cell_monitors, cell_address, CELL_MONITORS_REG_VOLTAGE_REF, voltage_ref)) {
      // write failed - reset the register value back to previous
      registers.cell_voltage_refs[cell_offset] = previous;
    }
  }
}

void isr_hardfault() {
  while (1) {
    led_blink();
    sleep_ms(500);
  }
}

int main() {
  // init
  stdio_init_all();

  exception_set_exclusive_handler(HARDFAULT_EXCEPTION, isr_hardfault);
  events_init();
  led_init();
  
  modbus_init(&modbus);
  modbus.unit_address = 1;
  modbus.registers = (uint16_t *)&registers;
  modbus.num_registers = sizeof(registers) / sizeof(uint16_t);
  modbus.write_callback = modbus_write_callback;

  onewire_init(&onewire);
  cell_monitors_init(&cell_monitors);

  // connect to cell monitors
  connect_loop();

  // main loop
  while (1) {
    if (!cell_monitors.connected) {
      puts("disconnected");
      connect_loop();
    }
    
    event_t event;
    while (events_dequeue(&event)) {
      switch (event.event_type) {
        case EVENT_TYPE_MEASURE:
          collect_measurements();
          break;
        case EVENT_TYPE_REGISTER:
          handle_register_written(event.reg_info.reg, event.reg_info.previous_value, event.reg_info.current_value);
          break;
      }
    }

    modbus_update(&modbus);
    sleep_ms(1);
  }
}
