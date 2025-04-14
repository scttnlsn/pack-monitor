#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "protection.h"
#include "config.h"
#include "temp.h"

void protection_init() {
    gpio_init(PROTECTION_CHG_PIN);
    gpio_set_dir(PROTECTION_CHG_PIN, GPIO_OUT);
    gpio_put(PROTECTION_CHG_PIN, 0);

    gpio_init(PROTECTION_DSG_PIN);
    gpio_set_dir(PROTECTION_DSG_PIN, GPIO_OUT);
    gpio_put(PROTECTION_DSG_PIN, 0);
}

void protection_update(registers_t *registers) {
    bool fault = false;
    bool ov = false; // over voltage
    bool uv = false; // under voltage
    bool ut = false; // under temp

    if (!(registers->status & STATUS_CONNECTED)) {
        fault = true;
    } else {
        uint16_t min_voltage = 0xFFFF;
        uint16_t max_voltage = 0;

        for (int i = 0; i < registers->num_cells; i++) {
            uint16_t cell_voltage = registers->cell_voltages[i];
            if (cell_voltage == 0) {
                // cell voltage may not have been read yet
                fault = true;
            }
            if (cell_voltage < min_voltage) {
                min_voltage = cell_voltage;
            }
            if (cell_voltage > max_voltage) {
                max_voltage = cell_voltage;
            }
        }

        if (!fault) {
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
    }

    temp_result_t temp = {
        .upper = registers->temp_upper,
        .lower = registers->temp_lower,
    };
    if (temp_c_int(temp) < CHARGE_CUTOFF_TEMP) {
        ut = true;
    }

    if (fault) {
        // disable both CHG and DSG to be safe
        gpio_put(PROTECTION_CHG_PIN, 0);
        gpio_put(PROTECTION_DSG_PIN, 0);
    } else {
        if (ov || ut) {
            gpio_put(PROTECTION_CHG_PIN, 0);
        } else {
            gpio_put(PROTECTION_CHG_PIN, 1);
        }
    
        if (uv) {
            gpio_put(PROTECTION_DSG_PIN, 0);
        } else {
            gpio_put(PROTECTION_DSG_PIN, 1);
        }
    }

    uint16_t protection = 0;
    if (ov) {
        protection |= PROTECTION_STATUS_OV;
    }
    if (uv) {
        protection |= PROTECTION_STATUS_UV;
    }
    if (ut) {
        protection |= PROTECTION_STATUS_UT;
    }
    if (fault) {
        protection |= PROTECTION_STATUS_FAULT;
    }
    registers->protection = protection;
}