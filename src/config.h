#ifndef __CONFIG_H__
#define __CONFIG_H__

// ------------------------------
// Battery config

// The number of series cells in the battery pack.
// There should be a cell monitor for each cell.
#define NUM_CELLS 4

// The minimum cell voltage (specified in mV) at which over-voltage protection is enabled.
#define OV_ENABLE 3550

// The maximum cell voltage (specified in mV) at which over-voltage protection is disabled.
// This only applies while OV is enabled.
#define OV_DISABLE 3350

// The maximum cell voltage (specified in mV) at which under-voltage proection is enabled.
#define UV_ENABLE 3000

// The minimum cell voltage (specified in mV) at which under-voltage protection is disabled.
// This only applies whiel UV is enabled.
#define UV_DISABLE 3100

// ------------------------------
// Hardware config

// I2C address of ADS1115 ADC
#define ADC_ADDRESS 0x48

// RX pin connected to cell monitors
#define RX_PIN 4

// TX pin connection to cell monitors
#define TX_PIN 5

// pin connected to charge relay
#define CHARGE_PIN 6

// pin connected to discharge relay
#define DISCHARGE_PIN 7

#endif
