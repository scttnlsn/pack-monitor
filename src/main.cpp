#include <Arduino.h>
#include <avr/wdt.h>
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

SoftwareSerial comm(RX_PIN, TX_PIN);
CellMonitors cell_monitors(comm);
Adc adc(ADC_ADDRESS);
Measurements measurements(&cell_monitors, &adc);
Protection protection(&measurements);
Capacity capacity(NOMINAL_CAPACITY);
Relay charge(CHARGE_PIN);
Relay discharge(DISCHARGE_PIN);
Timer timer;

void update() {
  if (!measurements.update()) {
    protection.fault();
  }

  int32_t net_current = measurements.charge_current() - measurements.discharge_current();
  capacity.update(net_current);

  uint8_t last_status = protection.status();
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
}

void log() {
  char name_buffer[16];
  char value_buffer[16];

  for (uint8_t i = 0; i < NUM_CELLS; i++) {
    uint16_t cell_voltage = measurements.cell_voltage(i);
    sprintf(name_buffer, "cell_%d", i + 1);
    sprintf(value_buffer, "%d", cell_voltage);
    serial::value(name_buffer, value_buffer);
  }

  uint16_t pack_voltage = measurements.pack_voltage();
  sprintf(value_buffer, "%d", pack_voltage);
  serial::value("pack_voltage", value_buffer);

  uint8_t status = protection.status();
  sprintf(value_buffer, "%d", status);
  serial::value("status", value_buffer);

  int32_t charge_current = measurements.charge_current();
  sprintf(value_buffer, "%ld", charge_current);
  serial::value("charge_current", value_buffer);

  int32_t discharge_current = measurements.discharge_current();
  sprintf(value_buffer, "%ld", discharge_current);
  serial::value("discharge_current", value_buffer);

  uint32_t charge = capacity.charge();
  sprintf(value_buffer, "%ld", charge);
  serial::value("charge", value_buffer);

  uint8_t soc = capacity.soc();
  sprintf(value_buffer, "%d", soc);
  serial::value("soc", value_buffer);
}

void setup() {
  charge.disable();
  discharge.disable();

  serial::init();

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
  timer.every(3000, log);

  serial::log("info", "main", "ready");
}

void loop() {
  timer.update();
}
