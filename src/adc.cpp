#include <Wire.h>
#include <Adafruit_ADS1015.h>

#include "adc.h"
#include "config.h"

namespace adc {
  Adafruit_ADS1115 adc(ADC_ADDRESS);

  void init() {
    adc.setGain(GAIN_ONE);
    adc.begin();
  }

  int16_t read_voltage(uint8_t channel) {
    int16_t result = adc.readADC_SingleEnded(0);

    // GAIN_ONE => 0.125mV per ADC step
    int16_t voltage = result / 8;

    return voltage;
  }
}
