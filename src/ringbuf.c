#include <string.h>

#include "ringbuf.h"

void ringbuf_init(ringbuf_t *ringbuf, void *buffer, uint32_t capacity, uint32_t size) {
  ringbuf->capacity = capacity;
  ringbuf->count = 0;
  ringbuf->size = size;
  ringbuf->buffer = buffer;
  ringbuf->head = buffer;
  ringbuf->tail = buffer;
}

bool ringbuf_push(ringbuf_t *ringbuf, const void *data) {
  if (ringbuf->count == ringbuf->capacity) {
    // full
    return false;
  }

  memcpy(ringbuf->head, data, ringbuf->size);

  ringbuf->head += ringbuf->size;
  if (ringbuf->head == ringbuf->buffer + (ringbuf->size * ringbuf->capacity)) {
    // wrap around
    ringbuf->head = ringbuf->buffer;
  }

  ringbuf->count++;
  return true;
}

bool ringbuf_pop(ringbuf_t *ringbuf, void *data) {
  if (ringbuf->count == 0) {
    // empty
    return false;
  }

  memcpy(data, ringbuf->tail, ringbuf->size);

  ringbuf->tail += ringbuf->size;
  if (ringbuf->tail == ringbuf->buffer + (ringbuf->size * ringbuf->capacity)) {
    // wrap around
    ringbuf->tail = ringbuf->buffer;
  }

  ringbuf->count--;
  return true;
}