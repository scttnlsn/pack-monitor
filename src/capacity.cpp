#include <Arduino.h>

#include "capacity.h"
#include "config.h"

// TODO: consider persisting _cc_net value to EEPROM

Capacity::Capacity(uint32_t capacity) : _capacity(capacity) {
  // charge unit is mA seconds
  _cc_counter = 0;
  _cc_charge = 0;
  _cc_discharge = 0;

  // default 100% SOC, will be accurate after full charge
  _cc_net = capacity;
}

void Capacity::begin() {
  _timestamp = millis();
  _cc_counter = 0;
}

void Capacity::reset() {
  _cc_net = _capacity;
}

void Capacity::update(int32_t charge_current, int32_t discharge_current) {
  // we assume the given current values have been constant since the last update

  uint32_t now = millis();
  float delta = now - _timestamp;

  if (_cc_counter + delta > CC_COUNTER_INTERVAL) {
    // start new counter window
    _cc_counter = 0;
    _cc_charge = 0;
    _cc_discharge = 0;
  }

  _cc_charge += charge_current * 1000 / delta;
  _cc_discharge += discharge_current * 1000 / delta;
  _cc_net += ((charge_current - discharge_current) * 1000 / delta);
  _cc_counter += delta;
  _timestamp = now;
}

uint32_t Capacity::cc_charge() {
  return _cc_charge;
}

uint32_t Capacity::cc_discharge() {
  return _cc_discharge;
}

uint32_t Capacity::cc_net() {
  return _cc_net;
}

uint32_t Capacity::cc_counter() {
  return _cc_counter;
}

uint32_t Capacity::soc() {
  // returns a value between 0 and 100 (percent) * 1000
  return ((uint64_t) _cc_net) * 100000 / _capacity;
}
