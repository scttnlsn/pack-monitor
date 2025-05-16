#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stddef.h>

// defines MAX_CELLS
#include "cell_monitors.h"

// bitmasks for `status` register
#define STATUS_CONNECTING (1 << 0)
#define STATUS_CONNECTED (1 << 1)

// bitmasks for `protection` register
#define PROTECTION_STATUS_OV (1 << 0)
#define PROTECTION_STATUS_UV (1 << 1)
#define PROTECTION_STATUS_UT (1 << 2)
#define PROTECTION_STATUS_FAULT (1 << 3)

// bitmasks for `error` register
#define ERROR_FAULT (1 << 0)
#define ERROR_WATCHDOG (1 << 1)
#define ERROR_CRC (1 << 2)
#define ERROR_TIMEOUT (1 << 3)
#define ERROR_NO_RESPONSE (1 << 4)
#define ERROR_MODBUS (1 << 5)

// packed and aligned so we can treat it as an array of uint16_t registers
typedef struct __attribute__((packed, aligned(sizeof(uint16_t)))) {
  uint16_t version;

  // bitmasked flags
  uint16_t status;
  uint16_t protection;
  uint16_t errors;

  uint16_t num_cells;

  // milliseconds
  uint16_t round_trip_time;

  // together these encode a signed floating point value
  uint16_t temp_upper;
  uint16_t temp_lower;

  uint16_t cell_voltages[MAX_CELLS];
} registers_t;

#endif