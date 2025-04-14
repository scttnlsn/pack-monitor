#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "protection.h"
#include "config.h"

bool ov = false;
bool uv = false;

void protection_init() {
    gpio_init(PROTECTION_CHG_PIN);
    gpio_set_dir(PROTECTION_CHG_PIN, GPIO_OUT);
    gpio_put(PROTECTION_CHG_PIN, 0);

    gpio_init(PROTECTION_DSG_PIN);
    gpio_set_dir(PROTECTION_DSG_PIN, GPIO_OUT);
    gpio_put(PROTECTION_DSG_PIN, 0);
}

void protection_update(registers_t *registers) {
    if (!registers->connected) {
        ov = true;
        uv = true;
    } else {
        uint16_t min_voltage = 0xFFFF;
        uint16_t max_voltage = 0;

        for (int i = 0; i < registers->num_cells; i++) {
            uint16_t cell_voltage = registers->cell_voltages[i];
            if (cell_voltage < min_voltage) {
                min_voltage = cell_voltage;
            }
            if (cell_voltage > max_voltage) {
                max_voltage = cell_voltage;
            }
        }

        if (min_voltage == 0xFFFF || max_voltage == 0) {
            ov = true;
            uv = true;
        } else {
            if (max_voltage > PROTECTION_OV_ENABLE) {
                ov = true;
            }

            if (min_voltage < PROTECTION_UV_ENABLE) {
                uv = true;
            }

            if (ov && max_voltage < PROTECTION_OV_ENABLE && max_voltage < PROTECTION_OV_DISABLE) {
                ov = false;
            }

            if (uv && min_voltage > PROTECTION_UV_ENABLE && min_voltage > PROTECTIOn_UV_DISABLE) {
                uv = false;
            }
        }
    }

    if (ov) {
        gpio_put(PROTECTION_CHG_PIN, 0);
    } else {
        gpio_put(PROTECTION_CHG_PIN, 1);
    }

    if (uv) {
        gpio_put(PROTECTION_DSG_PIN, 0);
    } else {
        gpio_put(PROTECTION_DSG_PIN, 1);
    }
    
    registers->ov = ov;
    registers->uv = uv;
}