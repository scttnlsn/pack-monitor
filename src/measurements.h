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
  void zero_current();
  uint16_t cell_voltage(uint8_t index);
  uint16_t pack_voltage();
  int32_t charge_current();
  int32_t discharge_current();

private:
  CellMonitors *_cell_monitors;
  Adc *_adc;

  // the most recent cell volage readings in mV
  uint16_t _cell_voltages[NUM_CELLS];

  // the most recent pack voltage reading in mV
  uint16_t _pack_voltage;

  // the most recent charge current reading in mA
  int32_t _charge_current;

  // the ADC value expected for 0mA charge current
  uint16_t _charge_adc_zero;

  // the most recent discharge current reading in mA
  int32_t _discharge_current;

  // the ADC value expected for 0mA discharge current
  uint16_t _discharge_adc_zero;
};

#endif
