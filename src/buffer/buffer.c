#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

void cobra_buffer_init(cobra_buffer_t *buffer, uint64_t size) {
    uv_mutex_init(&buffer->mutex_handle);
    buffer->size = size;
    buffer->head_pointer = malloc(size);
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer;
}

void cobra_buffer_deinit(cobra_buffer_t *buffer) {
    free(buffer->head_pointer);
}

uint64_t cobra_buffer_length(cobra_buffer_t *buffer) {
    uv_mutex_lock(&buffer->mutex_handle);
    uint64_t length = cobra__buffer_length(buffer);
    uv_mutex_unlock(&buffer->mutex_handle);

    return length;
}

uint64_t cobra_buffer_capacity(cobra_buffer_t *buffer) {
    uv_mutex_lock(&buffer->mutex_handle);
    uint64_t capacity = cobra__buffer_capacity(buffer);
    uv_mutex_unlock(&buffer->mutex_handle);

    return capacity;
}

uint64_t cobra__buffer_length(cobra_buffer_t *buffer) {
    return (uint64_t)(buffer->write_pointer - buffer->read_pointer);
}

uint64_t cobra__buffer_capacity(cobra_buffer_t *buffer) {
    return buffer->size - (uint64_t)(buffer->write_pointer - buffer->head_pointer);
}
