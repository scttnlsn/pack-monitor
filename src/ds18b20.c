#include "pico/stdlib.h"

#include "ds18b20.h"

uint32_t ds18b20_read_temp(onewire_t *onewire) {
  bool present = onewire_reset(onewire);
  if (!present) {
    return -1;
  }

  // skip ROM
  onewire_write(onewire, 0xCC);

  // read temp and put it on the scratchpad
  onewire_write(onewire, 0x44);

  sleep_ms(800);
  onewire_reset(onewire);

  // skip ROM
  onewire_write(onewire, 0xCC);

  // read scratchpad
  onewire_write(onewire, 0xBE);

  uint16_t temp_lsb = onewire_read(onewire);
  uint16_t temp_msb = onewire_read(onewire);

  // stop reading the rest of the scratchpad
  // TODO: read entire scratchpad and compare CRC
  onewire_reset(onewire);

  return (temp_msb << 16) | temp_lsb;
}

int32_t temp_c_int(uint32_t temp_raw) {
  uint16_t upper = temp_raw >> 16;
  uint16_t lower = temp_raw & 0xFFFF;

  int32_t temp = ((upper << 8) | lower) >> 4;
  uint8_t sign = upper & 0b10000000;
  if (sign) {
    temp *= -1;
  }

  return temp;
}