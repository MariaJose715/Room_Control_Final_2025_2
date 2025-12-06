#include "ring_buffer.h"
#include <string.h>

void ring_buffer_init(ring_buffer_t *rb, uint8_t *buffer, uint16_t capacity) {
    rb->buffer = buffer;
    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    memset(rb->buffer, 0, capacity);
}

bool ring_buffer_write(ring_buffer_t *rb, uint8_t data) {
    if (rb->count == rb->capacity) return false; // full

    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % rb->capacity;
    rb->count++;
    return true;
}

bool ring_buffer_read(ring_buffer_t *rb, uint8_t *data) {
    if (rb->count == 0) return false; // empty

    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count--;
    return true;
}

bool ring_buffer_is_empty(ring_buffer_t *rb) {
    return rb->count == 0;
}

bool ring_buffer_is_full(ring_buffer_t *rb) {
    return rb->count == rb->capacity;
}

uint16_t ring_buffer_count(ring_buffer_t *rb) {
    return rb->count;
}

void ring_buffer_flush(ring_buffer_t *rb) {
    rb->head = rb->tail = rb->count = 0;
    memset(rb->buffer, 0, rb->capacity);
}
