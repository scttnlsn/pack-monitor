#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "ringbuf.h"

#ifndef MODBUS_BUFFER_SIZE
#define MODBUS_BUFFER_SIZE 1024
#endif

typedef struct {
  uint8_t function_code;
  uint16_t address;
  uint16_t value;
} modbus_pdu_t;

typedef struct {
  uint8_t unit_address;
  modbus_pdu_t pdu;
  uint16_t crc;
} modbus_request_t;

typedef struct {
  uint8_t unit_address;
  uint8_t buffer[MODBUS_BUFFER_SIZE];
  ringbuf_t ringbuf;
  modbus_request_t request;
  uint8_t error;

  uint16_t *registers;
  uint32_t num_registers;

  // accepts the register index, the previous register value and the current register value
  void (*write_callback)(uint16_t reg, uint16_t previous, uint16_t current);
} modbus_t;

void modbus_init(modbus_t *modbus);
void modbus_update(modbus_t *modbus);

#endif
