#ifndef __CELL_MONITORS_H__
#define __CELL_MONITORS_H__

#include <stdint.h>
#include <stdbool.h>

#define CELL_MONITORS_REG_ADDRESS 0x1
#define CELL_MONITORS_REG_VOLTAGE_REF 0x2
#define CELL_MONITORS_REG_VOLTAGE 0x3
#define CELL_MONITORS_REG_TEMP 0x4
#define CELL_MONITORS_REG_BALANCE 0x5

typedef struct {
  bool connected;
  uint16_t num_cells;
} cell_monitors_t;

void cell_monitors_init(cell_monitors_t *cell_monitors);
bool cell_monitors_connect(cell_monitors_t *cell_monitors);
bool cell_monitors_read(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg, uint16_t *value);
bool cell_monitors_write(cell_monitors_t *cell_monitors, uint8_t cell_address, uint8_t reg, uint16_t value);

#endif
