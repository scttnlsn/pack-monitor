#ifndef __RELAY_H__
#define __RELAY_H__

#include <stdint.h>

class Relay {
public:
  Relay(uint8_t pin);
  void enable();
  void disable();

private:
  uint8_t _pin;
};

#endif
