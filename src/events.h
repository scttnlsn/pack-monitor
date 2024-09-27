#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "pico/stdlib.h"

typedef enum {
    EVENT_TYPE_MEASURE,
    EVENT_TYPE_REGISTER,
} event_type_t;

typedef struct {
    event_type_t event_type;
    union {
        // EVENT_TYPE_REGISTER
        struct {
            uint16_t reg;
            uint16_t previous_value;
            uint16_t current_value;
        } reg_info;
    };
} event_t;

void events_init();
bool events_enqueue(event_t event);
bool events_dequeue(event_t *event);

#endif
