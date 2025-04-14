#ifndef __PROTECTION_H__
#define __PROTECTION_H__

#include <stdint.h>
#include <stdbool.h>

#include "registers.h"

void protection_init();
void protection_update(registers_t *registers);

#endif
