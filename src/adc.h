#ifndef __ADC_H__
#define __ADC_H__

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <stdint.h>

#define ADC_GAIN GAIN_ONE

// GAIN_ONE => 0.125mV per ADC step
#define ADC_VOLTAGE_SCALER 8

class Adc {
public:
  Adc(uint8_t address);
  uint16_t read_raw(uint8_t channel);
  uint16_t read_voltage(uint8_t channel);

private:
  Adafruit_ADS1115 _adc;
};

#endif
