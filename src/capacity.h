#ifndef __CAPACITY_H__
#define __CAPACITY_H__

#include <stdint.h>

class Capacity {
public:
  Capacity(uint32_t capacity);
  void begin();
  void reset();
  void update(int32_t current);
  uint32_t charge();
  uint8_t soc();

private:
  uint32_t _capacity;
  uint32_t _timestamp;
  uint32_t _charge;
};

#endif
