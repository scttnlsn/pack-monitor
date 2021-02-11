#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "cell_monitors.h"

#define UART_ID uart1
#define UART_BAUD_RATE 9600
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define ADDRESS_BROADCAST 0x0

#define REG_ADDRESS 0x1
#define REG_VOLTAGE_REF 0x2
#define REG_VOLTAGE 0x3
#define REG_TEMP 0x4
#define REG_BALANCE 0x5

#define PACKET_LENGTH 4
#define PACKET_TIMEOUT 100

typedef struct {
  uint8_t address;
  uint8_t request;
  uint8_t reg;
  uint8_t write;
  uint16_t value;
} packet_t;

static bool send(packet_t *packet);
static bool receive(packet_t *packet);
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
  packet.reg = REG_ADDRESS;
  packet.write = 1;
  packet.value = 1;

  if (!send(&packet)) {
    return false;
  }
  if (!receive(&packet)) {
    return false;
  }

  cell_monitors->connected = true;
  cell_monitors->num_cells = packet.value - 1;
  return true;
}

bool cell_monitors_read_voltage(cell_monitors_t *cell_monitors, uint8_t cell_address, uint16_t *voltage) {
  packet_t packet;
  packet.address = cell_address;
  packet.request = 1;
  packet.reg = REG_VOLTAGE;
  packet.write = 0;
  packet.value = 0;

  if (!send(&packet)) {
    return false;
  }
  if (!receive(&packet)) {
    return false;
  }

  *voltage = packet.value;
  return true;
}

// static functions

static bool send(packet_t *packet) {
  uint8_t buffer[PACKET_LENGTH + 1];
  encode(buffer, packet);

  uint8_t crc = crc8(buffer, PACKET_LENGTH);
  buffer[PACKET_LENGTH] = crc;

  uart_write_blocking(UART_ID, buffer, PACKET_LENGTH + 1);
  return true;
}

static bool receive(packet_t *packet) {
  uint8_t buffer[PACKET_LENGTH + 1];
  uart_read_blocking(UART_ID, buffer, PACKET_LENGTH + 1);

  uint8_t crc = crc8(buffer, PACKET_LENGTH);
  if (buffer[PACKET_LENGTH] != crc) {
    return false;
  }

  decode(buffer, packet);
  return true;
}

// 4 byte packet structure (msb to lsb):
//
// address (7 bits)
// request=1/response=0 flag (1 bit)
// reg (7 bits)
// write=1/read=0 flag (1 bit)
// value (16 bits)

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
