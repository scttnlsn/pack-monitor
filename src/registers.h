#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stddef.h>

// defines MAX_CELLS
#include "cell_monitors.h"

// packed and aligned so we can treat it as an array of uint16_t registers
typedef struct __attribute__((packed, aligned(sizeof(uint32_t)))) {
  uint16_t version;
  uint16_t connected;
  uint16_t error_code;
  uint16_t num_cells;
  uint16_t round_trip_time;

  // this encodes a signed floating point temp
  union {
    uint32_t temp;
    struct {
      uint16_t upper;
      uint16_t lower;
    } temp_parts;
  };

  uint16_t cell_voltages[MAX_CELLS];
  uint16_t cell_voltage_refs[MAX_CELLS];
} registers_t;

#endif