#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t count;
  uint32_t size;
  uint8_t *buffer;
  uint8_t *head;
  uint8_t *tail;
} ringbuf_t;

void ringbuf_init(ringbuf_t *ringbuf, void *buffer, uint32_t size);
bool ringbuf_push(ringbuf_t *ringbuf, uint8_t byte);
bool ringbuf_pop(ringbuf_t *ringbuf, uint8_t *byte);

#endif
