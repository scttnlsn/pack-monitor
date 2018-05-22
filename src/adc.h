#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>

namespace adc {
  void init();
  int16_t read_voltage(uint8_t channel);
}

#endif
