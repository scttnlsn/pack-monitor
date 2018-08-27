#ifndef __CAPACITY_H__
#define __CAPACITY_H__

#include <stdint.h>

class Capacity {
public:
  Capacity(uint32_t capacity);
  void begin();
  void reset();
  void update(int32_t charge_current, int32_t discharge_current);
  uint32_t soc();
  uint32_t cc_charge();
  uint32_t cc_discharge();
  uint32_t cc_net();
  uint32_t cc_counter();

private:
  uint32_t _capacity;
  uint32_t _timestamp;

  uint32_t _cc_counter;
  uint32_t _cc_charge;
  uint32_t _cc_discharge;
  uint32_t _cc_net;
};

#endif
