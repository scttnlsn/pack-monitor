#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

#include "led.h"

// onboard LED
const uint8_t LED_PIN = 25;

void led_init() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
}

void led_blink() {
  gpio_put(LED_PIN, 1);
  sleep_ms(50);
  gpio_put(LED_PIN, 0);
  sleep_ms(50);
}

void _pwm_handler() {
  static int value = 0;
  static bool increase = true;

  pwm_clear_irq(pwm_gpio_to_slice_num(LED_PIN));

  if (increase) {
    // brightness increasing
    value++;
    if (value > 255) {
      value = 255;
      increase = false;
    }
  } else {
    // brightness decreasing
    value--;
    if (value < 0) {
      value = 0;
      increase = true;
    }
  }

  pwm_set_gpio_level(LED_PIN, value * value);
}

void led_pulse_start() {
  gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(LED_PIN);

  pwm_clear_irq(slice_num);
  pwm_set_irq_enabled(slice_num, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, _pwm_handler);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, 4.f);
  pwm_init(slice_num, &config, true);
}

void led_pulse_stop() {
  uint slice_num = pwm_gpio_to_slice_num(LED_PIN);
  pwm_set_enabled(slice_num, false);
  pwm_clear_irq(slice_num);

  gpio_set_function(LED_PIN, GPIO_FUNC_SIO);
  gpio_set_dir(LED_PIN, GPIO_OUT);
}