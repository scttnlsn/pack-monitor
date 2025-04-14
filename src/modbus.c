#include <stdio.h>
#include <string.h>
#include <pico/stdio.h>
#include <pico/error.h>

#include "modbus.h"
#include "events.h"
#include "led.h"

static void modbus_read(modbus_t *modbus);
static void modbus_parse_request(modbus_t *modbus);
static void modbus_send(modbus_t *modbus, uint8_t *message, uint32_t len);
static void modbus_handle_error(modbus_t *modbus);
static void modbus_handle_request(modbus_t *modbus);
static void modbus_read_holding_register(modbus_t *modbus);
static void modbus_write_single_register(modbus_t *modbus);
static uint16_t modbus_crc16(const uint8_t *data, uint8_t len);

void modbus_init(modbus_t *modbus) {
  ringbuf_init(&modbus->ringbuf, modbus->buffer, sizeof(modbus->buffer), 1);

  // clear stdin
  while (1) {
    int res = getchar_timeout_us(0);
    if (res == PICO_ERROR_TIMEOUT) {
      break;
    }
  }
}

void modbus_update(modbus_t *modbus) {
  modbus->request.unit_address = 0;
  modbus->request.pdu.function_code = 0;
  modbus->error = 0;

  modbus_read(modbus);

  if (modbus->error) {
    modbus_handle_error(modbus);
    return;
  }

  // packet is at least 8 bytes
  if (modbus->ringbuf.count < 8) {
    return;
  }

  modbus_parse_request(modbus);
  modbus_handle_request(modbus);
}

// static functions

static void modbus_read(modbus_t *modbus) {
  while (1) {
    int res = getchar_timeout_us(0);
    if (res == PICO_ERROR_TIMEOUT) {
      break;
    }

    uint8_t byte = (uint8_t) res;
    uint8_t buffer[1];
    buffer[0] = (uint8_t) res;
    if (!ringbuf_push(&modbus->ringbuf, (void *)buffer)) {
      // error, not enough room in ringbuf

      // TODO: define enum for errors
      modbus->error = -1;
    }
  }
}

static void modbus_parse_request(modbus_t *modbus) {
  uint8_t buffer[8];
  for (int i = 0; i < 8; i++) {
    ringbuf_pop(&modbus->ringbuf, buffer + i);
  }

  modbus->request.unit_address = buffer[0];
  modbus->request.pdu.function_code = buffer[1];
  modbus->request.pdu.address = (buffer[2] << 8) | buffer[3];
  modbus->request.pdu.value = (buffer[4] << 8) | buffer[5];
  modbus->request.crc = (buffer[6] << 8) | buffer[7];

  // TODO: check crc
}

static void modbus_send(modbus_t *modbus, uint8_t *message, uint32_t len) {
  uint8_t buffer[1024];
  uint32_t index = 0;

  buffer[index++] = modbus->unit_address;

  for (int i = 0; i < len; i++) {
    buffer[index++] = message[i];
  }

  uint16_t crc = modbus_crc16(buffer, index);

  // crc is sent little endian
  buffer[index++] = crc & 0xFF;
  buffer[index++] = (crc >> 8) & 0xFF;

  for (int i = 0; i < index; i++) {
    putc(buffer[i], stdout);
  }

  fflush(stdout);
}

static void modbus_handle_error(modbus_t *modbus) {
  uint8_t buffer[2];
  buffer[0] = 0x80 + modbus->request.pdu.function_code;
  buffer[1] = 0x04; // device failure
  modbus_send(modbus, buffer, sizeof(buffer));
}

static void modbus_handle_request(modbus_t *modbus) {
  if (modbus->unit_address != modbus->request.unit_address) {
    return;
  }

  uint8_t buffer[2];

  switch (modbus->request.pdu.function_code) {
  case 3:
    modbus_read_holding_register(modbus);
    break;
  case 6:
    modbus_write_single_register(modbus);
    break;
  default:
    buffer[0] = 0x80 + modbus->request.pdu.function_code;
    buffer[1] = 0x01; // illegal function
    modbus_send(modbus, buffer, sizeof(buffer));
    break;
  }
}

static void modbus_read_holding_register(modbus_t *modbus) {
  uint16_t register_idx = modbus->request.pdu.address - 1;
  uint16_t register_count = modbus->request.pdu.value;
  uint8_t byte_count = register_count * 2;

  uint8_t response[byte_count + 2];
  uint8_t index = 0;
  response[index++] = modbus->request.pdu.function_code;
  response[index++] = byte_count;

  if (register_idx < 0 || register_idx > modbus->num_registers) {
    modbus_handle_error(modbus);
    return;
  }

  for (int i = 0; i < register_count; i++) {
    uint16_t register_value = modbus->registers[register_idx + i];
    response[index++] = (register_value >> 8) & 0xFF;
    response[index++] = register_value & 0xFF;
  }

  modbus_send(modbus, response, sizeof(response));
}

static void modbus_write_single_register(modbus_t *modbus) {
  uint16_t register_idx = modbus->request.pdu.address - 1;
  uint16_t register_value = modbus->request.pdu.value;

  uint16_t previous_value = modbus->registers[register_idx];
  modbus->registers[register_idx] = register_value;

  uint8_t response[5];
  uint8_t index = 0;
  response[index++] = modbus->request.pdu.function_code;
  response[index++] = (modbus->request.pdu.address >> 8) & 0xFF;
  response[index++] = modbus->request.pdu.address & 0xFF;
  response[index++] = (register_value >> 8) & 0xFF;
  response[index++] = register_value & 0xFF;

  modbus_send(modbus, response, sizeof(response));

  if (previous_value != register_value) {
    event_t event = {
      .event_type = EVENT_TYPE_REGISTER_UPDATED,
      .reg_info = {
        .reg = register_idx,
        .previous_value = previous_value,
        .current_value = register_value,
      },
    };
  events_enqueue(event);
  }
}

static uint16_t modbus_crc16(const uint8_t *data, uint8_t len) {
  uint16_t crc = 0xFFFF;
  
  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  
  return crc;
}
