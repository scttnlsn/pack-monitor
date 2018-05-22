#ifndef __ADC_H__
#define __ADC_H__

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <stdint.h>

class Adc {
public:
  Adc(uint8_t address);
  int16_t read_voltage(uint8_t channel);

private:
  Adafruit_ADS1115 _adc;
};

#endif
