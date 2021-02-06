#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "led.h"

const uint8_t LED_PIN = 25;

void led_init() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
}

void led_blink() {
  gpio_put(LED_PIN, 1);
  sleep_ms(100);
  gpio_put(LED_PIN, 0);
  sleep_ms(100);
}
