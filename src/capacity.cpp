#include <Arduino.h>

#include "capacity.h"

// TODO: consider persisting _charge value to EEPROM

Capacity::Capacity(uint32_t capacity) : _capacity(capacity) {
  // charge unit is mA seconds
  // default 50% SOC, will be accurate after full charge
  _charge = capacity / 2;
}

void Capacity::begin() {
  _timestamp = millis();
}

void Capacity::reset() {
  _charge = _capacity;
}

void Capacity::update(int32_t current) {
  // current is the net charging current (charging current - discharge current)
  // we assume the given current has been constant since the last update

  uint32_t now = millis();
  float delta = now - _timestamp;

  _charge += (current * 1000 / delta);
  _timestamp = now;
}

uint32_t Capacity::charge() {
  return _charge;
}

uint8_t Capacity::soc() {
  // returns a value between 0 and 100 (percent)
  return ((uint64_t) _charge) * 100 / _capacity;
}
