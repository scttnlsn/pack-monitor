#ifndef __MEASUREMENTS_H__
#define __MEASUREMENTS_H__

#include <stdint.h>

#include "adc.h"
#include "cell_monitors.h"
#include "config.h"

class Measurements {
public:
  Measurements(CellMonitors *cell_monitors, Adc *adc);
  bool update();
  uint16_t cell_voltage(uint8_t index);
  uint16_t pack_voltage();

private:
  CellMonitors *_cell_monitors;
  Adc *_adc;
  uint16_t _cell_voltages[NUM_CELLS];
  uint16_t _pack_voltage;
};

#endif
