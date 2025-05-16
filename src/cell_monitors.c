#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/uart.h"

#include "cell_monitors.h"
#include "config.h"
#include "events.h"

#define CELL_MONITORS_BAUD_RATE 9600
#define ADDRESS_BROADCAST 0x0
#define PACKET_LENGTH 6 // bytes
#define REQUEST_QUEUE_LENGTH 32

static void handle_response(cell_monitors_t *cell_monitors);
void send(cell_monitors_t *cell_monitors, cell_monitors_packet_t packet);
static void decode(uint8_t *buffer, cell_monitors_packet_t *packet);
static void encode(uint8_t *buffer, cell_monitors_packet_t *packet);
static uint8_t crc8(const uint8_t *buffer, uint32_t length);

void cell_monitors_init(cell_monitors_t *cell_monitors, uart_inst_t *uart, uint rx_pin, uint tx_pin) {
  cell_monitors->uart = uart;
  cell_monitors->connected = false;
  cell_monitors->connecting = false;
  cell_monitors->num_cells = 0;
  cell_monitors->packet_id = 0;
  cell_monitors->waiting = false;
  memset(&cell_monitors->current_request, 0, sizeof(cell_monitors_request_t));

  for (int cell_address = 1; cell_address <= MAX_CELLS; cell_address++) {
    cell_state_t cell_state = {
      .cell_address = cell_address,
      .voltage = 0,
      .last_read_at = 0,
    };
    cell_monitors->cell_states[cell_address - 1] = cell_state;
  }

  ringbuf_init(
    &cell_monitors->uart_ringbuf,
    cell_monitors->uart_buffer,
    sizeof(cell_monitors->uart_buffer),
    1
  );

  queue_init(
    &cell_monitors->request_queue,
    sizeof(cell_monitors_request_t),
    REQUEST_QUEUE_LENGTH
  );

  uart_init(cell_monitors->uart, CELL_MONITORS_BAUD_RATE);
  gpio_set_function(rx_pin, GPIO_FUNC_UART);
  gpio_set_function(tx_pin, GPIO_FUNC_UART);
}

void cell_monitors_update(cell_monitors_t *cell_monitors) {
  // copy all available bytes from the uart to the uart buffer
  while (uart_is_readable(cell_monitors->uart)) {
    uint8_t buffer[1];
    buffer[0] = uart_getc(cell_monitors->uart);
    ringbuf_push(&cell_monitors->uart_ringbuf, buffer);
  }

  // handle response for each packet in the uart buffer
  while (cell_monitors->uart_ringbuf.count >= PACKET_LENGTH) {
    handle_response(cell_monitors);
  }

  if (!cell_monitors->waiting) {
    // send out the next request if we're not currently waiting for a response
    cell_monitors_request_t request;
    if (queue_try_remove(&cell_monitors->request_queue, &request)) {
      // there's a pending request
      send(cell_monitors, request.packet);

      request.sent_at = get_absolute_time();
      cell_monitors->current_request = request;
      cell_monitors->waiting = true;
    }
  } else {
    // waiting for a response

    absolute_time_t last_sent_at = cell_monitors->current_request.sent_at;
    int64_t waiting_ms = absolute_time_diff_us(last_sent_at, get_absolute_time()) / 1000;

    if (waiting_ms > REQUEST_TIMEOUT) {
      cell_monitors->current_request.error |= CELL_MONITORS_ERROR_TIMEOUT;
      cell_monitors_disconnect(cell_monitors);
    }
  }
}

bool cell_monitors_connect(cell_monitors_t *cell_monitors) {
  cell_monitors->connecting = true;

  cell_monitors_packet_t packet;
  packet.address = ADDRESS_BROADCAST;
  packet.request = 1;
  packet.reg = CELL_MONITORS_REG_ADDRESS;
  packet.write = 1;
  packet.value = 1;

  cell_monitors_request_t request = {
    .packet = packet,
    .sent_at = 0,
    .received_at = 0,
    .error = 0,
  };

  if (!queue_try_add(&cell_monitors->request_queue, &request)) {
    return false;
  }
}

void cell_monitors_disconnect(cell_monitors_t *cell_monitors) {
  // reset state
  cell_monitors->connecting = false;
  cell_monitors->connected = false;
  cell_monitors->waiting = false;

  for (int i = 0; i < MAX_CELLS; i++) {
    cell_monitors->cell_states[i].voltage = 0;
    cell_monitors->cell_states[i].last_read_at = 0;
  }

  // clear request queue
  cell_monitors_request_t request;
  while (queue_try_remove(&cell_monitors->request_queue, &request));

  // clear the uart buffer
  uint8_t buffer[1];
  while (ringbuf_pop(&cell_monitors->uart_ringbuf, buffer));

  // trigger disconnect event
  event_t event = {
    .event_type = EVENT_TYPE_DISCONNECTED,
  };
  events_enqueue(event);
}

bool cell_monitors_read(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg) {
  cell_monitors_packet_t packet;
  packet.id = cell_monitors->packet_id++;
  packet.address = cell_address;
  packet.request = 1;
  packet.reg = reg;
  packet.write = 0;
  packet.value = 0;

  cell_monitors_request_t request = {
    .packet = packet,
    .sent_at = 0,
    .received_at = 0,
    .error = 0,
  };

  return queue_try_add(&cell_monitors->request_queue, &request);
}

bool cell_monitors_write(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg, uint16_t value) {
  cell_monitors_packet_t packet;
  packet.id = cell_monitors->packet_id++;
  packet.address = cell_address;
  packet.request = 1;
  packet.reg = reg;
  packet.write = 1;
  packet.value = value;

  cell_monitors_request_t request = {
    .packet = packet,
    .sent_at = 0,
    .received_at = 0,
    .error = 0,
  };

  return queue_try_add(&cell_monitors->request_queue, &request);
}

// static functions

void send(cell_monitors_t *cell_monitors, cell_monitors_packet_t packet) {
  uint8_t buffer[PACKET_LENGTH];
  encode(buffer, &packet);

  uint8_t crc = crc8(buffer, PACKET_LENGTH - 1);
  buffer[PACKET_LENGTH - 1] = crc;

  uart_write_blocking(cell_monitors->uart, buffer, PACKET_LENGTH);

  // printf("id: %d, ", packet.id);
  // printf("address: %d, ", packet.address);
  // printf("request: %d, ", packet.request);
  // printf("reg: %d, ", packet.reg);
  // printf("write: %d, ", packet.write);
  // printf("value: %d\n", packet.value);
}

static void handle_response(cell_monitors_t *cell_monitors) {
  if (!cell_monitors->waiting) {
    // there was no pending request but we still received some activity on the UART
    // - this could happen after we disconnect but there's still an in-flight request

    // clear the uart buffer
    uint8_t buffer[1];
    while (ringbuf_pop(&cell_monitors->uart_ringbuf, buffer));
    return;
  }

  uint8_t buffer[PACKET_LENGTH];
  for (uint8_t bytes_read = 0; bytes_read < PACKET_LENGTH; bytes_read++) {
    if (!ringbuf_pop(&cell_monitors->uart_ringbuf, buffer + bytes_read)) {
      // we expected data in the UART buffer but not enough was present
      cell_monitors->current_request.error |= CELL_MONITORS_ERROR_TIMEOUT;
      cell_monitors_disconnect(cell_monitors);
      return;
    }
  }

  cell_monitors_request_t request = cell_monitors->current_request;
  request.received_at = get_absolute_time();

  uint8_t crc = crc8(buffer, PACKET_LENGTH - 1);
  if (buffer[PACKET_LENGTH - 1] != crc) {
    request.error |= CELL_MONITORS_ERROR_CRC;
    return;
  }

  // TODO: check packet id matches

  cell_monitors_packet_t response_packet;
  decode(buffer, &response_packet);

  // printf("id: %d, ", response_packet.id);
  // printf("address: %d, ", response_packet.address);
  // printf("request: %d, ", response_packet.request);
  // printf("reg: %d, ", response_packet.reg);
  // printf("write: %d, ", response_packet.write);
  // printf("value: %d\n", response_packet.value);

  if (response_packet.address == 0) {
    // request was a broadcast
    if (response_packet.request != 1) {
      // since this was a broadcast request there should be no response
      request.error = CELL_MONITORS_ERROR_NO_RESPONSE;
    } else {
      cell_monitors->connected = true;
      cell_monitors->connecting = false;
      cell_monitors->num_cells = response_packet.value - 1;

      event_t event = {
        .event_type = EVENT_TYPE_CONNECTED,
      };
      events_enqueue(event);
    }
  } else {
    // normal request
    if (response_packet.request != 0) {
      // no response (packet was just forwarded through the daisy chain)
      request.error = CELL_MONITORS_ERROR_NO_RESPONSE;
    } else {
      uint8_t cell_index = response_packet.address - 1;
      cell_monitors->cell_states[cell_index].voltage = response_packet.value;
      cell_monitors->cell_states[cell_index].last_read_at = get_absolute_time();

      event_t event = {
        .event_type = EVENT_TYPE_CELL_UPDATED,
        .cell_address = response_packet.address,
      };
      events_enqueue(event);
    }
  }

  cell_monitors->waiting = false;
}

// packet structure:
//
//   7    6    5    4    3    2    1    0
// +----+----+----+----+----+----+----+----+
// | PACKET ID                             | 0
// +----+----+----+----+----+----+----+----+
// | ADDRESS                          | RR | 1
// +----+----+----+----+----+----+----+----+
// | REG                              | RW | 2
// +----+----+----+----+----+----+----+----+
// | VALUE UPPER                           | 3
// +----+----+----+----+----+----+----+----+
// | VALUE LOWER                           | 4
// +----+----+----+----+----+----+----+----+
// | CRC                                   | 5
// +----+----+----+----+----+----+----+----+
//
// PACKET ID (8 bits)
//  - incrementing ID
// ADDRESS (7 bits)
//  - address of the cell module
// RR (1 bit)
//  - request/response flag
//  - request = 1
//  - response = 0
// REG (7 bits)
//  - cell module register
// RW (1 bit)
//  - read/write flag
//  - write = 1
//  - read = 0
// VALUE (16 bits)
// CRC (8 bits)

static void decode(uint8_t *buffer, cell_monitors_packet_t *packet) {
  packet->id = buffer[0];
  packet->address = buffer[1] >> 1;
  packet->request = buffer[1] & 0x1;
  packet->reg = buffer[2] >> 1;
  packet->write = buffer[2] & 0x1;
  packet->value = (buffer[3] << 8) | buffer[4];
}

static void encode(uint8_t *buffer, cell_monitors_packet_t *packet) {
  buffer[0] = packet->id;
  buffer[1] = (packet->address << 1) | (packet->request & 0x1);
  buffer[2] = (packet->reg << 1) | (packet->write & 0x1);
  buffer[3] = packet->value >> 8;
  buffer[4] = packet->value & 0xFF;
}

static uint8_t crc8(const uint8_t *buffer, uint32_t length) {
  uint8_t crc = 0;

  for (uint8_t i = 0; i < length; i++) {
    uint8_t data = crc ^ buffer[i];

    for (uint8_t j = 0; j < 8; j++ ) {
      if (data & 0x80) {
        data <<= 1;
        data ^= 0x07;
      } else {
        data <<= 1;
      }
    }

    crc = data;
  }

  return crc;
}
