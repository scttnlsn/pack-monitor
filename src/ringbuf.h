#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  // the max number of elements that can be held in the ringbuf
  uint32_t capacity;

  // the current count of elements in the ringbuf
  uint32_t count;

  // the size of each element (in bytes)
  uint32_t size;

  // the raw storage buffer
  void *buffer;

  // pointer to head
  void *head;

  // pointer to tail
  void *tail;
} ringbuf_t;

void ringbuf_init(ringbuf_t *ringbuf, void *buffer, uint32_t capacity, uint32_t size);
bool ringbuf_push(ringbuf_t *ringbuf, const void *data);
bool ringbuf_pop(ringbuf_t *ringbuf, void *data);

#endif