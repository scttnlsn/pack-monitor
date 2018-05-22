#include <Arduino.h>

#include "cell_monitors.h"
#include "config.h"
#include "serial.h"
#include "utils.h"

#define ADDRESS_BROADCAST 0x0

#define REG_ADDRESS 0x1
#define REG_VOLTAGE_REF 0x2
#define REG_VOLTAGE 0x3
#define REG_TEMP 0x4
#define REG_BALANCE 0x5

#define PACKET_LENGTH 4
#define PACKET_TIMEOUT 100

void encode(uint8_t *buffer, packet_t *packet);
void decode(uint8_t *buffer, packet_t *packet);

CellMonitors::CellMonitors(Stream &stream) : _stream(stream) {
  _num_cells = 0;
}

bool CellMonitors::connect() {
  packet_t packet;
  packet.address = ADDRESS_BROADCAST;
  packet.request = 1;
  packet.reg = REG_ADDRESS;
  packet.write = 1;
  packet.value = 1;

  if (!send(&packet)) {
    serial::log("error", "cell_monitors", "packet not sent");
    return false;
  }

  if (!receive(&packet)) {
    serial::log("error", "cell_monitors", "packet not received");
    return false;
  }

  _num_cells = packet.value - 1;

  if (_num_cells != NUM_CELLS) {
    serial::log("error", "cell_monitors", "invalid num cells");
    return false;
  }

  return true;
}

bool CellMonitors::read_voltage(uint8_t cell_address, uint16_t *voltage) {
  packet_t packet;
  packet.address = cell_address;
  packet.request = 1;
  packet.reg = REG_VOLTAGE;
  packet.write = 0;
  packet.value = 0;

  if (!send(&packet)) {
    serial::log("error", "cell_monitors", "failed to send voltage read packet");
    return false;
  }

  if (!receive(&packet)) {
    serial::log("error", "cell_monitors", "did not receive voltage packet");
    return false;
  }

  *voltage = packet.value;

  return true;
}

bool CellMonitors::send(packet_t *packet) {
  uint8_t buffer[PACKET_LENGTH];
  encode(buffer, packet);

  for (uint8_t i = 0; i < PACKET_LENGTH; i++) {
    if (_stream.write(buffer[i]) != 1) {
      // failed to write byte
      return false;
    }
  }

  if (_stream.write(utils::crc8(buffer, PACKET_LENGTH)) != 1) {
    return false;
  }

  _stream.flush();

  return true;
}

bool CellMonitors::receive(packet_t *packet) {
  uint32_t start = millis();
  uint8_t buffer[PACKET_LENGTH + 1];

  for (uint8_t i = 0; i < (PACKET_LENGTH + 1); i++) {
    while (!_stream.available()) {
      if (millis() - start > PACKET_TIMEOUT) {
        serial::log("error", "cell_monitors", "packet timeout");
        return false;
      }
    }

    buffer[i] = _stream.read();
  }

  uint8_t crc = utils::crc8(buffer, PACKET_LENGTH);
  if (buffer[PACKET_LENGTH] != crc) {
    serial::log("error", "cell_monitors", "invalid CRC byte");
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

void decode(uint8_t *buffer, packet_t *packet) {
  packet->address = buffer[0] >> 1;
  packet->request = buffer[0] & 0x1;
  packet->reg = buffer[1] >> 1;
  packet->write = buffer[1] & 0x1;
  packet->value = (buffer[2] << 8) | buffer[3];
}

void encode(uint8_t *buffer, packet_t *packet) {
  buffer[0] = (packet->address << 1) | (packet->request & 0x1);
  buffer[1] = (packet->reg << 1) | (packet->write & 0x1);
  buffer[2] = packet->value >> 8;
  buffer[3] = packet->value & 0xFF;
}
