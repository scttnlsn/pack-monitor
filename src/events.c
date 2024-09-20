#include <stdio.h>
#include "pico/util/queue.h"

#include "events.h"

#define QUEUE_LENGTH 32

queue_t queue;

void events_init() {
    queue_init(&queue, sizeof(event_t), QUEUE_LENGTH);
};

bool events_enqueue(event_t event) {
    return queue_try_add(&queue, &event);
}

bool events_dequeue(event_t *event) {
    return queue_try_remove(&queue, event);
}