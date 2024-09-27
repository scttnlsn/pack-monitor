#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/uart.h"

#include "cell_monitors.h"

#define UART_ID uart1
#define UART_BAUD_RATE 9600
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define ADDRESS_BROADCAST 0x0

#define PACKET_LENGTH 4
#define PACKET_TIMEOUT_MS 2000

typedef struct {
  uint8_t address;
  uint8_t request;
  uint8_t reg;
  uint8_t write;
  uint16_t value;
} packet_t;

static void send(packet_t *packet);
static bool recv(packet_t *packet);
static void decode(uint8_t *buffer, packet_t *packet);
static void encode(uint8_t *buffer, packet_t *packet);
static uint8_t crc8(const uint8_t *buffer, uint32_t length);

void cell_monitors_init(cell_monitors_t *cell_monitors) {
  uart_init(UART_ID, UART_BAUD_RATE);

  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  cell_monitors->connected = false;
  cell_monitors->num_cells = 0;
}

bool cell_monitors_connect(cell_monitors_t *cell_monitors) {
  packet_t packet;
  packet.address = ADDRESS_BROADCAST;
  packet.request = 1;
  packet.reg = CELL_MONITORS_REG_ADDRESS;
  packet.write = 1;
  packet.value = 1;

  send(&packet);
  if (!recv(&packet) != 0) {
    return false;
  }

  if (packet.request != 1) {
    // since this was a broadcast request there should be no response
    puts("unexpected response from broadcast request");
    return false;
  }

  cell_monitors->connected = true;
  cell_monitors->num_cells = packet.value - 1;
  return true;
}

bool cell_monitors_read(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg, uint16_t *value) {
  packet_t packet;
  packet.address = cell_address;
  packet.request = 1;
  packet.reg = reg;
  packet.write = 0;
  packet.value = 0;

  send(&packet);
  if (!recv(&packet)) {
    cell_monitors->connected = false;
    return false;
  }

  if (packet.request != 0) {
    // no response (packet was forwarded all the way through the daisy chain)
    puts("no packet response");
    return false;
  }

  *value = packet.value;
  return true;
}

bool cell_monitors_write(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg, uint16_t value) {
  packet_t packet;
  packet.address = cell_address;
  packet.request = 1;
  packet.reg = reg;
  packet.write = 1;
  packet.value = value;

  send(&packet);
  if (!recv(&packet)) {
    cell_monitors->connected = false;
    return false;
  }

  if (packet.request != 0) {
    // no response (packet was forwarded all the way through the daisy chain)
    puts("no packet response");
    return false;
  }

  if (packet.value != value) {
    puts("unexpected value in response");
    return false;
  }

  return true;
}

// static functions

static void send(packet_t *packet) {
  // Clear UART RX in case there are and old packets in the buffer.
  // We want to make sure the next response pertains to this sent packet.
  while (uart_is_readable(UART_ID)) {
    uart_getc(UART_ID);
  }

  uint8_t buffer[PACKET_LENGTH + 1];
  encode(buffer, packet);

  uint8_t crc = crc8(buffer, PACKET_LENGTH);
  buffer[PACKET_LENGTH] = crc;

  uart_write_blocking(UART_ID, buffer, PACKET_LENGTH + 1);
}

static bool recv(packet_t *packet) {
  uint8_t buffer[PACKET_LENGTH + 1];

  uint64_t start = time_us_64();
  uint8_t bytes_read = 0;
  while (bytes_read < PACKET_LENGTH + 1) {
    uint64_t now = time_us_64();
    
    if (now - start > PACKET_TIMEOUT_MS * 1000) {
      // timeout
      puts("packet recv timeout");
      return false;
    }

    if (!uart_is_readable(UART_ID)) {
      continue;
    }

    buffer[bytes_read] = uart_getc(UART_ID);
    bytes_read++;
  }

  uint8_t crc = crc8(buffer, PACKET_LENGTH);
  if (buffer[PACKET_LENGTH] != crc) {
    puts("packet CRC error");
    return false;
  }

  decode(buffer, packet);
  return true;
}

// 4 byte packet structure:
//
//   7    6    5    4    3    2    1    0
// +----+----+----+----+----+----+----+----+
// | ADDRESS                          | RR | 0
// +----+----+----+----+----+----+----+----+
// | REG                              | RW | 1
// +----+----+----+----+----+----+----+----+
// | VALUE UPPER                           | 2
// +----+----+----+----+----+----+----+----+
// | VALUE LOWER                           | 3
// +----+----+----+----+----+----+----+----+
//
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

static void decode(uint8_t *buffer, packet_t *packet) {
  packet->address = buffer[0] >> 1;
  packet->request = buffer[0] & 0x1;
  packet->reg = buffer[1] >> 1;
  packet->write = buffer[1] & 0x1;
  packet->value = (buffer[2] << 8) | buffer[3];
}

static void encode(uint8_t *buffer, packet_t *packet) {
  buffer[0] = (packet->address << 1) | (packet->request & 0x1);
  buffer[1] = (packet->reg << 1) | (packet->write & 0x1);
  buffer[2] = packet->value >> 8;
  buffer[3] = packet->value & 0xFF;
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
