#include "pico/stdlib.h"

#include "temp.h"

void temp_init(temp_t *temp, onewire_t *onewire) {
  temp->onewire = onewire;
  temp->sent_at = 0;
  temp->waiting = false;
}

bool temp_start_read(temp_t *temp) {
  bool present = onewire_reset(temp->onewire);
  if (!present) {
    return false;
  }

  // skip ROM
  onewire_write(temp->onewire, 0xCC);

  // read temp and put it on the scratchpad
  onewire_write(temp->onewire, 0x44);

  temp->waiting = true;
  temp->sent_at = get_absolute_time();
}

bool temp_update(temp_t *temp, temp_result_t *result) {
  if (!temp->waiting) {
    return false;
  }

  absolute_time_t now = get_absolute_time();
  int64_t wait_time = absolute_time_diff_us(temp->sent_at, now) / 1000;

  if (wait_time < 800) {
    // wait at least 800ms for read to finish
    return false;
  }

  onewire_reset(temp->onewire);

  // skip ROM
  onewire_write(temp->onewire, 0xCC);

  // read scratchpad
  onewire_write(temp->onewire, 0xBE);

  uint16_t lower = onewire_read(temp->onewire);
  uint16_t upper = onewire_read(temp->onewire);

  // stop reading the rest of the scratchpad
  // TODO: read entire scratchpad and compare CRC
  onewire_reset(temp->onewire);

  result->upper = upper;
  result->lower = lower;
  temp->waiting = false;

  return true;
}

int32_t temp_c_int(temp_result_t temp) {
  int32_t c_int = ((temp.upper << 8) | temp.lower) >> 4;
  
  uint8_t sign = temp.upper & 0b10000000;
  if (sign) {
    c_int *= -1;
  }

  return c_int;
}