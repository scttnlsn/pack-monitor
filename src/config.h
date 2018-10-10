#ifndef __CONFIG_H__
#define __CONFIG_H__

// When a communication error occurs, we wait this long
// before triggering a fault
#define ERROR_TIMEOUT 5000

// ------------------------------
// Battery config

// The number of series cells in the battery pack.
// There should be a cell monitor for each cell.
#define NUM_CELLS 8

// The minimum cell voltage (specified in mV) at which over-voltage protection is enabled.
#define OV_ENABLE 3550

// The maximum cell voltage (specified in mV) at which over-voltage protection is disabled.
// This only applies while OV is enabled.
#define OV_DISABLE 3350

// The maximum cell voltage (specified in mV) at which under-voltage proection is enabled.
#define UV_ENABLE 3000

// The minimum cell voltage (specified in mV) at which under-voltage protection is disabled.
// This only applies while UV is enabled.
#define UV_DISABLE 3100

// The nominal battery capacity (specified in mA seconds)
#define NOMINAL_CAPACITY 360000000 // 100Ah

// The maximum error tolerated between the pack voltage reading
// and the sum of the cell voltages (specified in mV)
#define MAX_VOLTAGE_ERROR 200

// The time interval (in milliseconds) during which coulomb-counting values are summed
#define CC_COUNTER_INTERVAL 3600000 // 1 hour

// ------------------------------
// Hardware config

// I2C address of ADS1115 ADC
#define ADC_ADDRESS 0x48

// ADS1115 channels
#define ADC_CHANNEL_PACK_VOLTAGE 0
#define ADC_CHANNEL_CHARGE_CURRENT 2
#define ADC_CHANNEL_DISCHARGE_CURRENT 3

// ACS758 sensitivity (mV / A)
#define ACS758_SENSITIVITY 20

// RX pin connected to cell monitors
#define RX_PIN 4

// TX pin connection to cell monitors
#define TX_PIN 5

// pin connected to charge relay
#define CHARGE_PIN 6

// pin connected to discharge relay
#define DISCHARGE_PIN 7

// voltage divider scaler for reading pack voltage
// 100k / 10k divider
#define PACK_VOLTAGE_DIVIDER 110000 / 10000

#endif
