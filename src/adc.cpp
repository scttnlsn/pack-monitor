#include "adc.h"

#define NUM_SAMPLES 4

Adc::Adc(uint8_t address) : _adc(address) {
  _adc.setGain(ADC_GAIN);
  _adc.begin();
}

uint16_t Adc::read_raw(uint8_t channel) {
  uint32_t sum = 0;

  for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
    sum += _adc.readADC_SingleEnded(channel);
  }

  return (uint16_t) (sum / NUM_SAMPLES);
}

uint16_t Adc::read_voltage(uint8_t channel) {
  uint16_t result = read_raw(channel);
  uint16_t voltage = result / ADC_VOLTAGE_SCALER;
  return voltage;
}
