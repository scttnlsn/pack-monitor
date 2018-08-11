#include "config.h"
#include "measurements.h"
#include "protection.h"

Protection::Protection(Measurements *measurements) : _measurements(measurements) {
  _ov = false;
  _uv = false;
  _fault = false;
}

void Protection::update() {
  uint16_t max_voltage = 0;
  uint16_t min_voltage = 0xFFFF;
  uint16_t sum_voltage = 0;

  for (uint8_t i = 0; i < NUM_CELLS; i++) {
    uint16_t voltage = _measurements->cell_voltage(i);
    sum_voltage += voltage;

    if (voltage > max_voltage) {
      max_voltage = voltage;
    }

    if (voltage < min_voltage) {
      min_voltage = voltage;
    }
  }

  if (max_voltage > OV_ENABLE) {
    _ov = true;
  }

  if (min_voltage < UV_ENABLE) {
    _uv = true;
  }

  if (_ov && max_voltage < OV_ENABLE && max_voltage < OV_DISABLE) {
    _ov = false;
  }

  if (_uv && min_voltage > UV_ENABLE && min_voltage > UV_DISABLE) {
    _uv = false;
  }

  if (abs((int16_t) sum_voltage - (int16_t) _measurements->pack_voltage()) >= MAX_VOLTAGE_ERROR) {
    // we've exceeded max allowable error in voltage measurements
    _fault = true;
  }
}

void Protection::fault() {
  _fault = true;
}

uint8_t Protection::status() {
  uint8_t status = 0;

  if (_ov) {
    status |= PROTECTION_STATUS_OV;
  }

  if (_uv) {
    status |= PROTECTION_STATUS_UV;
  }

  if (_fault) {
    status |= PROTECTION_STATUS_FAULT;
  }

  return status;
}
