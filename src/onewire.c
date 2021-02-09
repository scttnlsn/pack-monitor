#include "pico/stdlib.h"

#include "onewire.h"

// protocol overview:
// https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html

static void onewire_write_bit(onewire_t *onewire, bool bit);
static bool onewire_read_bit(onewire_t *onewire);

void onewire_init(onewire_t *onewire) {
  gpio_init(onewire->pin);
}

bool onewire_reset(onewire_t *onewire) {
  gpio_set_dir(onewire->pin, GPIO_OUT);
  gpio_put(onewire->pin, 0);
  sleep_us(480);

  gpio_set_dir(onewire->pin, GPIO_IN);
  sleep_us(70);

  // val = 0 if device is present
  // val = 1 if no device is present
  bool val = gpio_get(onewire->pin);
  sleep_us(410);

  return !val;
}

void onewire_write(onewire_t *onewire, uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t bit = (byte >> i) & 0x01;
    onewire_write_bit(onewire, bit);
  }
}

uint8_t onewire_read(onewire_t *onewire) {
  uint8_t byte = 0x00;

  for (uint8_t i = 0; i < 8; i++) {
    if (onewire_read_bit(onewire)) {
      byte |= (0x01 << i);
    }
  }

  return byte;
}

// static functions

static void onewire_write_bit(onewire_t *onewire, bool bit) {
  if (bit) {
    // sending a 1
    gpio_set_dir(onewire->pin, GPIO_OUT);
    gpio_put(onewire->pin, 0);
    sleep_us(6);

    gpio_set_dir(onewire->pin, GPIO_IN);
    sleep_us(64);
  } else {
    // sending a 0
    gpio_set_dir(onewire->pin, GPIO_OUT);
    gpio_put(onewire->pin, 0);
    sleep_us(60);

    gpio_set_dir(onewire->pin, GPIO_IN);
    sleep_us(10);
  }
}

static bool onewire_read_bit(onewire_t *onewire) {
  gpio_set_dir(onewire->pin, GPIO_OUT);
  gpio_put(onewire->pin, 0);
  sleep_us(6);

  gpio_set_dir(onewire->pin, GPIO_IN);
  sleep_us(9);

  bool val = gpio_get(onewire->pin);
  sleep_us(55);

  return val;
}
