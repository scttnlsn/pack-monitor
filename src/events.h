#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "pico/stdlib.h"

typedef struct {
    uint8_t event_type;
    void *data;
} event_t;

void events_init();
bool events_enqueue(event_t event);
bool events_dequeue(event_t *event);

#endif
