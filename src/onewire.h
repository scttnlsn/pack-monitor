#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t pin;
} onewire_t;

void onewire_init(onewire_t *onewire);
bool onewire_reset(onewire_t *onewire);
void onewire_write(onewire_t *onewire, uint8_t byte);
uint8_t onewire_read(onewire_t *onewire);

#endif
