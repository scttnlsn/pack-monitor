#include <stdlib.h>

#include "adc.h"
#include "config.h"
#include "measurements.h"
#include "serial.h"

// convert ADC reading to current (mA)
#define CURRENT(adc) ((adc) * 1000 / ADC_VOLTAGE_SCALER / ACS758_SENSITIVITY)

Measurements::Measurements(CellMonitors *cell_monitors, Adc *adc) : _cell_monitors(cell_monitors), _adc(adc) {
  for (uint8_t i = 0; i < NUM_CELLS; i++) {
    _cell_voltages[i] = 0;
  }

  _pack_voltage = 0;
  _charge_current = 0;
  _charge_adc_zero = 0;
  _discharge_current = 0;
  _discharge_adc_zero = 0;
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

  uint16_t adc_voltage = _adc->read_voltage(ADC_CHANNEL_PACK_VOLTAGE);
  _pack_voltage = adc_voltage * PACK_VOLTAGE_DIVIDER;

  uint16_t adc_charge_current = _adc->read_raw(ADC_CHANNEL_CHARGE_CURRENT);
  _charge_current = CURRENT((int32_t) adc_charge_current - (int32_t) _charge_adc_zero);

  uint16_t adc_discharge_current = _adc->read_raw(ADC_CHANNEL_DISCHARGE_CURRENT);
  _discharge_current = CURRENT((int32_t) adc_discharge_current - (int32_t) _discharge_adc_zero);

  return true;
}

void Measurements::zero_current() {
  _charge_adc_zero = _adc->read_raw(ADC_CHANNEL_CHARGE_CURRENT);
  _discharge_adc_zero = _adc->read_raw(ADC_CHANNEL_DISCHARGE_CURRENT);
}

uint16_t Measurements::cell_voltage(uint8_t index) {
  return _cell_voltages[index];
}

uint16_t Measurements::pack_voltage() {
  return _pack_voltage;
}

int32_t Measurements::charge_current() {
  return _charge_current;
}

int32_t Measurements::discharge_current() {
  return _discharge_current;
}
