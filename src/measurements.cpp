#include <stdlib.h>

#include "adc.h"
#include "config.h"
#include "measurements.h"
#include "serial.h"

Measurements::Measurements(CellMonitors *cell_monitors, Adc *adc) : _cell_monitors(cell_monitors), _adc(adc) {
  for (uint8_t i = 0; i < NUM_CELLS; i++) {
    _cell_voltages[i] = 0;
  }

  _pack_voltage = 0;
}

bool Measurements::update() {
  for (uint8_t i = 0; i < NUM_CELLS; i++) {
    uint8_t cell_address = i + 1;
    uint16_t voltage;

    if (!_cell_monitors->read_voltage(cell_address, &voltage)) {
      serial::log("error", "measurements", "could not read cell voltage");
      return false;
    }

    _cell_voltages[i] = voltage;
  }

  int16_t adc_voltage = _adc->read_voltage(0);
  _pack_voltage = (uint16_t) (adc_voltage * 11); // 100k - 10k divider

  return true;
}

uint16_t Measurements::cell_voltage(uint8_t index) {
  return _cell_voltages[index];
}

uint16_t Measurements::pack_voltage() {
  return _pack_voltage;
}
