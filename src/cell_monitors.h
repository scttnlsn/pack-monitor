#ifndef __CELL_MONITORS_H__
#define __CELL_MONITORS_H__

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "ringbuf.h"

// cell monitors use 7 bit addresses
#define MAX_CELLS 128

#define CELL_MONITORS_REG_ADDRESS 0x1
#define CELL_MONITORS_REG_VOLTAGE_REF 0x2
#define CELL_MONITORS_REG_VOLTAGE 0x3
#define CELL_MONITORS_REG_BALANCE 0x4

typedef struct {
  uint8_t id;
  uint8_t address;
  uint8_t request;
  uint8_t reg;
  uint8_t write;
  uint16_t value;
} cell_monitors_packet_t;

typedef struct {
  cell_monitors_packet_t packet;
  absolute_time_t sent_at;
  absolute_time_t received_at;
  uint8_t error;
} cell_monitors_request_t;

typedef struct {
  uint8_t cell_address;
  uint16_t voltage; // mV
  uint16_t voltage_ref;
  absolute_time_t last_read_at;
} cell_state_t;

typedef struct {
  uart_inst_t *uart;

  bool connected;
  bool connecting;
  bool waiting; // waiting for response

  uint8_t num_cells;
  cell_state_t cell_states[MAX_CELLS];

  // buffers received bytes from the UART
  uint8_t uart_buffer[4096];
  ringbuf_t uart_ringbuf;

  uint8_t packet_id;
  queue_t request_queue;

  // set when waiting = true
  cell_monitors_request_t current_request;
} cell_monitors_t;

void cell_monitors_init(cell_monitors_t *cell_monitors, uart_inst_t *uart, uint rx_pin, uint tx_pin);
void cell_monitors_update(cell_monitors_t *cell_monitors);
bool cell_monitors_connect(cell_monitors_t *cell_monitors);
void cell_monitors_disconnect(cell_monitors_t *cell_monitors);
bool cell_monitors_read(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg);
bool cell_monitors_write(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg, uint16_t value);

#endif
