#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

void cobra_buffer_clear(cobra_buffer_t *buffer) {
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer;
}

void cobra_buffer_resize(cobra_buffer_t *buffer, uint64_t new_size) {
    if (buffer->size >= new_size)
        return;

    buffer->size = new_size;
    buffer->head_pointer = realloc(buffer->head_pointer, new_size);
}

void cobra_buffer_fragment(cobra_buffer_t *buffer) {
    int length = cobra_buffer_length(buffer);
    if (length == 0 || buffer->read_pointer == buffer->head_pointer)
        return;

    memmove(buffer->head_pointer, buffer->read_pointer, length);
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer + length;
}

bool cobra_buffer_equals(cobra_buffer_t *buffer, const uint8_t *data, uint64_t length) {
    if (cobra_buffer_length(buffer) != length)
        return false;

    return memcmp(buffer->read_pointer, data, length) == 0;
}