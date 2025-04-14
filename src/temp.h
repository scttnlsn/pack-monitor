#ifndef __TEMP_H__
#define __TEMP_H__

#include "pico/time.h"

#include "onewire.h"

typedef struct {
    uint16_t upper;
    uint16_t lower;
} temp_result_t;

typedef struct {
    onewire_t *onewire;
    bool waiting;
    absolute_time_t sent_at;
} temp_t;

void temp_init(temp_t *temp, onewire_t *onewire);
bool temp_start_read(temp_t *temp);
bool temp_update(temp_t *temp, temp_result_t *result);

int32_t temp_c_int(temp_result_t temp);

#endif
