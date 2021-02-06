#include <string.h>

#include "ringbuf.h"

void ringbuf_init(ringbuf_t *ringbuf, void *buffer, uint32_t size) {
  ringbuf->buffer = buffer;
  ringbuf->size = size;

  ringbuf->head = ringbuf->buffer;
  ringbuf->tail = ringbuf->buffer;

  ringbuf->count = 0;
}

bool ringbuf_push(ringbuf_t *ringbuf, uint8_t byte) {
  if (ringbuf->count == ringbuf->size) {
    // full
    return false;
  }

  *ringbuf->head = byte;
  ringbuf->head++;

  if (ringbuf->head == ringbuf->buffer + ringbuf->size) {
    // wrap around
    ringbuf->head = ringbuf->buffer;
  }

  ringbuf->count++;
  return true;
}

bool ringbuf_pop(ringbuf_t *ringbuf, uint8_t *byte) {
  if (ringbuf->count == 0) {
    // empty
    return false;
  }

  *byte = *ringbuf->tail;
  ringbuf->tail++;

  if (ringbuf->tail == ringbuf->buffer + ringbuf->size) {
    // wrap around
    ringbuf->tail = ringbuf->buffer;
  }

  ringbuf->count--;
  return true;
}
