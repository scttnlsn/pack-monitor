#include <Arduino.h>
#include <SoftwareSerial.h>

#include "adc.h"
#include "cell_monitors.h"
#include "config.h"
#include "serial.h"

SoftwareSerial comm(RX_PIN, TX_PIN);
CellMonitors cell_monitors(comm);

void setup() {
  serial::init();
  adc::init();

  cell_monitors.begin();
}

void loop() {
  int16_t voltage = adc::read_voltage(0);

  char buffer[10];
  sprintf(buffer, "%d", voltage);
  serial::value("voltage_0", buffer);

  delay(2000);
}
