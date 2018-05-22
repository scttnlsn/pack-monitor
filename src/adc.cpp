#include "adc.h"

Adc::Adc(uint8_t address) : _adc(address) {
  _adc.setGain(GAIN_ONE);
  _adc.begin();
}

int16_t Adc::read_voltage(uint8_t channel) {
  int16_t result = _adc.readADC_SingleEnded(0);

  // GAIN_ONE => 0.125mV per ADC step
  int16_t voltage = result / 8;

  return voltage;
}
