#include <Arduino.h>

#include "relay.h"

Relay::Relay(uint8_t pin) : _pin(pin) {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void Relay::enable() {
  digitalWrite(_pin, HIGH);
}

void Relay::disable() {
  digitalWrite(_pin, LOW);
}
