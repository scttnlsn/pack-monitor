#ifndef __CELL_MONITORS_H__
#define __CELL_MONITORS_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  bool connected;
  uint16_t num_cells;
} cell_monitors_t;

void cell_monitors_init(cell_monitors_t *cell_monitors);
bool cell_monitors_connect(cell_monitors_t *cell_monitors);
bool cell_monitors_read_voltage(cell_monitors_t *cell_monitors, uint8_t cell_address, uint16_t *voltage);

#endif
