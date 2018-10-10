#include <Arduino.h>
#include <avr/wdt.h>
#include <ModbusSlave.h>
#include <SoftwareSerial.h>
#include <Timer.h>

#include "adc.h"
#include "capacity.h"
#include "cell_monitors.h"
#include "config.h"
#include "measurements.h"
#include "protection.h"
#include "relay.h"
#include "serial.h"

#define NUM_REGISTERS 15 + NUM_CELLS

typedef enum {
  REGISTER_STATUS = 0,
  REGISTER_CHARGE_CURRENT_MSB,
  REGISTER_CHARGE_CURRENT_LSB,
  REGISTER_DISCHARGE_CURRENT_MSB,
  REGISTER_DISCHARGE_CURRENT_LSB,
  REGISTER_CC_CHARGE_MSB,
  REGISTER_CC_CHARGE_LSB,
  REGISTER_CC_DISCHARGE_MSB,
  REGISTER_CC_DISCHARGE_LSB,
  REGISTER_CC_NET_MSB,
  REGISTER_CC_NET_LSB,
  REGISTER_CC_COUNTER_MSB,
  REGISTER_CC_COUNTER_LSB,
  REGISTER_SOC_MSB,
  REGISTER_SOC_LSB,

  REGISTER_CELL_VOLTAGES
} register_t;

SoftwareSerial comm(RX_PIN, TX_PIN);
CellMonitors cell_monitors(comm);
Adc adc(ADC_ADDRESS);
Measurements measurements(&cell_monitors, &adc);
Protection protection(&measurements);
Capacity capacity(NOMINAL_CAPACITY);
Relay charge(CHARGE_PIN);
Relay discharge(DISCHARGE_PIN);
Timer timer;
Modbus modbus(Serial, 1, -1);

uint16_t registers[NUM_REGISTERS];

uint8_t read_registers(uint8_t fc, uint16_t address, uint16_t length) {
  if (address + length > NUM_REGISTERS) {
    return STATUS_ILLEGAL_DATA_ADDRESS;
  }

  for (uint8_t i = 0; i < length; i++) {
    modbus.writeRegisterToBuffer(i, registers[i]);
  }

  return STATUS_OK;
}

void update_registers() {
  uint8_t status = protection.status();
  registers[REGISTER_STATUS] = status;

  int32_t charge_current = measurements.charge_current();
  registers[REGISTER_CHARGE_CURRENT_MSB] = (charge_current >> 16) & 0xFFFF;
  registers[REGISTER_CHARGE_CURRENT_LSB] = charge_current & 0xFFFF;

  int32_t discharge_current = measurements.discharge_current();
  registers[REGISTER_DISCHARGE_CURRENT_MSB] = (discharge_current >> 16) & 0xFFFF;
  registers[REGISTER_DISCHARGE_CURRENT_LSB] = discharge_current & 0xFFFF;

  uint32_t cc_charge = capacity.cc_charge();
  registers[REGISTER_CC_CHARGE_MSB] = (cc_charge >> 16) & 0xFFFF;
  registers[REGISTER_CC_CHARGE_LSB] = cc_charge & 0xFFFF;

  uint32_t cc_discharge = capacity.cc_discharge();
  registers[REGISTER_CC_DISCHARGE_MSB] = (cc_discharge >> 16) & 0xFFFF;
  registers[REGISTER_CC_DISCHARGE_LSB] = cc_discharge & 0xFFFF;

  uint32_t cc_net = capacity.cc_net();
  registers[REGISTER_CC_NET_MSB] = (cc_net >> 16) & 0xFFFF;
  registers[REGISTER_CC_NET_LSB] = cc_net & 0xFFFF;

  uint32_t cc_counter = capacity.cc_counter();
  registers[REGISTER_CC_COUNTER_MSB] = (cc_counter >> 16) & 0xFFFF;
  registers[REGISTER_CC_COUNTER_LSB] = cc_counter & 0xFFFF;

  uint32_t soc = capacity.soc();
  registers[REGISTER_SOC_MSB] = (soc >> 16) & 0xFFFF;
  registers[REGISTER_SOC_LSB] = soc & 0xFFFF;

  for (uint8_t i = 0; i < NUM_CELLS; i++) {
    uint16_t cell_voltage = measurements.cell_voltage(i);
    registers[REGISTER_CELL_VOLTAGES + i] = cell_voltage;
  }
}

void update() {
  uint8_t last_status = protection.status();

  if (last_status & PROTECTION_STATUS_ERROR) {
    if (cell_monitors.connect() && measurements.update()) {
      protection.clear_error();
    }
  } else {
    if (!measurements.update()) {
      protection.error();
    }
  }

  capacity.update(measurements.charge_current(), measurements.discharge_current());

  protection.update();
  uint8_t status = protection.status();

  if (status & PROTECTION_STATUS_FAULT) {
    charge.disable();
    discharge.disable();
  } else {
    if (status & PROTECTION_STATUS_OV) {
      if (!(last_status & PROTECTION_STATUS_OV)) {
        capacity.reset();
      }

      charge.disable();
    } else {
      charge.enable();
    }

    if (status & PROTECTION_STATUS_UV) {
      discharge.disable();
    } else {
      discharge.enable();
    }
  }

  update_registers();
}

void setup() {
  charge.disable();
  discharge.disable();

  serial::init();

  modbus.cbVector[CB_READ_REGISTERS] = read_registers;
  modbus.begin(115200);

  comm.begin(9600);

  if (!cell_monitors.connect()) {
    protection.fault();
    serial::log("error", "main", "cell monitors not initialized");
    return;
  }

  delay(1000);
  measurements.zero_current();
  capacity.begin();
  update();

  timer.every(1000, update);

  // 1 second watchdog
  wdt_enable(WDTO_1S);

  serial::log("info", "main", "ready");
}

void loop() {
  timer.update();

  uint8_t bytes = modbus.poll();
  if (bytes > 0) {
    Serial.flush();
  }

  wdt_reset();
}
