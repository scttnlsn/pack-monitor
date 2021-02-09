#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "onewire.h"

uint32_t ds18b20_read_temp(onewire_t *onewire);

#endif
