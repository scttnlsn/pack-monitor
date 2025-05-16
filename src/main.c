#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/exception.h"
#include "pico/time.h"
#include "hardware/watchdog.h"

#include "config.h"
#include "events.h"
#include "led.h"
#include "modbus.h"
#include "onewire.h"
#include "temp.h"
#include "cell_monitors.h"
#include "registers.h"
#include "protection.h"

registers_t registers;
onewire_t onewire = { .pin = ONEWIRE_PIN };
modbus_t modbus;
cell_monitors_t cell_monitors;
repeating_timer_t timer;
temp_t temp;

static void handle_connected();
static void handle_disconnected();
static void handle_interval();
static void handle_register_updated();
static void handle_cell_updated();

bool timer_callback(repeating_timer_t *rt) {
  event_t event = {
    .event_type = EVENT_TYPE_INTERVAL,
  };
  events_enqueue(event);

  // keep repeating
  return true;
}

void isr_hardfault() {
  watchdog_disable();

  while (1) {
    led_blink();
    led_blink();
    led_blink();
    sleep_ms(1000);
  }
}

void connect() {
  led_pulse_start();
  if (!cell_monitors_connect(&cell_monitors)) {
    // could not enqueue connection request
    registers.errors |= ERROR_FAULT;
    led_pulse_stop();
  } else {
    registers.status |= STATUS_CONNECTING;
  }
}

int main() {
  watchdog_enable(3000, false);
  exception_set_exclusive_handler(HARDFAULT_EXCEPTION, isr_hardfault);

  if (watchdog_caused_reboot()) {
    registers.errors |= ERROR_WATCHDOG;
  }

  protection_init();
  
  stdio_init_all();
  // we're sending binary data over stdio
  stdio_set_translate_crlf(&stdio_usb, false);

  events_init();
  led_init();
  onewire_init(&onewire);
  temp_init(&temp, &onewire);

  memset(&registers, 0, sizeof(registers_t));
  registers.version = 1;
  
  modbus_init(&modbus);
  modbus.unit_address = 1;
  modbus.registers = (uint16_t *)&registers;
  modbus.num_registers = sizeof(registers) / sizeof(uint16_t);

  cell_monitors_init(
    &cell_monitors,
    uart1,
    CELL_MONITORS_UART_RX_PIN,
    CELL_MONITORS_UART_TX_PIN
  );

  add_repeating_timer_ms(
    CELL_MONITORS_INTERVAL,
    timer_callback,
    NULL,
    &timer
  );

  connect();

  // main loop
  while (true) {
    watchdog_update();

    // process any events from the queue
    event_t event;
    while (events_dequeue(&event)) {
      switch (event.event_type) {
        case EVENT_TYPE_CONNECTED:
          handle_connected(&event);
          break;
        case EVENT_TYPE_DISCONNECTED:
          handle_disconnected(&event);
          break;
        case EVENT_TYPE_INTERVAL:
          handle_interval(&event);
          break;
        case EVENT_TYPE_REGISTER_UPDATED:
          handle_register_updated(&event);
          break;
        case EVENT_TYPE_CELL_UPDATED:
          handle_cell_updated(&event);
          break;
      }
    }

    // update various sub-systems
    cell_monitors_update(&cell_monitors);
    protection_update(&registers);
    modbus_update(&modbus);

    temp_result_t temp_result;
    if (temp_update(&temp, &temp_result)) {
      registers.temp_upper = temp_result.upper;
      registers.temp_lower = temp_result.lower;
    }

    // update registers
    if (cell_monitors.connected) {
      if (cell_monitors.current_request.error) {
        if (cell_monitors.current_request.error & CELL_MONITORS_ERROR_CRC) {
          registers.errors |= ERROR_CRC;
        } else {
          registers.errors &= ~ERROR_CRC;
        }

        if (cell_monitors.current_request.error & CELL_MONITORS_ERROR_TIMEOUT) {
          registers.errors |= ERROR_TIMEOUT;
        } else {
          registers.errors &= ~ERROR_TIMEOUT;
        }

        if (cell_monitors.current_request.error & CELL_MONITORS_ERROR_NO_RESPONSE) {
          registers.errors |= ERROR_NO_RESPONSE;
        } else {
          registers.errors &= ~ERROR_NO_RESPONSE;
        }
      }
    }

    if (modbus.error) {
      registers.errors |= ERROR_MODBUS;
    } else {
      registers.errors &= ~ERROR_MODBUS;
    }

    sleep_ms(10);
  }
}

// event handlers

static void handle_connected(event_t *event) {
  led_pulse_stop();
  registers.status |= STATUS_CONNECTED;
  registers.status &= ~STATUS_CONNECTING;
  registers.num_cells = cell_monitors.num_cells;
}

static void handle_disconnected(event_t *event) {
  registers.status &= ~STATUS_CONNECTED;
  for (int i = 0; i < registers.num_cells; i++) {
    registers.cell_voltages[i] = 0;
  }

  connect();
}

static void handle_interval(event_t *event) {
  if (!cell_monitors.connected) {
    return;
  }

  // 2 blinks signals the start of a measurement cycle
  led_blink();
  led_blink();

  // enqueue voltage read request for each cell
  uint8_t num_cells = cell_monitors.num_cells;
  for (uint8_t cell_address = 1; cell_address <= num_cells; cell_address++) {
    cell_monitors_read(
        &cell_monitors,
        cell_address,
        CELL_MONITORS_REG_VOLTAGE
    );
  }

  // start new temp read
  if (!temp.waiting) {
    temp_start_read(&temp);
  }
}

static void handle_register_updated(event_t *event) {
  // a register was written - we may need to write the update to the cell monitors
  uint16_t reg = event->reg_info.reg;
  uint16_t previous_value = event->reg_info.previous_value;
  uint16_t current_value = event->reg_info.current_value;

  // offsetof returns bytes so we divide by 2
  uint16_t offset = offsetof(registers_t, cell_voltages) / 2;

  if (offset <= reg && reg < offset + MAX_CELLS) {
    // a cell voltage reference was updated
    uint16_t cell_offset = reg - offset;
    uint16_t cell_address = cell_offset + 1;
    uint16_t cell_voltage = registers.cell_voltages[cell_offset];

    cell_monitors_write(
      &cell_monitors,
      cell_address,
      CELL_MONITORS_REG_VOLTAGE,
      cell_voltage
    );
  }
}

static void handle_cell_updated(event_t *event) {
  // cell state was updated
  uint8_t cell_address = event->cell_address;

  uint8_t cell_index = cell_address - 1;
  registers.cell_voltages[cell_index] = cell_monitors.cell_states[cell_index].voltage;

  absolute_time_t sent_at = cell_monitors.current_request.sent_at;
  absolute_time_t received_at = cell_monitors.current_request.received_at;
  registers.round_trip_time = (uint16_t)absolute_time_diff_us(sent_at, received_at) / (uint16_t)1000;

  // 1 blink indicates 1 measurement received
  led_blink();
}