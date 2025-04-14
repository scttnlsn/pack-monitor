#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "onewire.h"

uint32_t ds18b20_read_temp(onewire_t *onewire);
int32_t temp_c_int(uint32_t temp_raw);

#endif
