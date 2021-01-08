#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

void cobra_buffer_resize(cobra_buffer_t *buffer, uint64_t new_size) {
    uv_mutex_lock(&buffer->mutex_handle);

    if (buffer->size >= new_size) {
        uv_mutex_unlock(&buffer->mutex_handle);
        return;
    }

    buffer->size = new_size;
    buffer->head_pointer = realloc(buffer->head_pointer, new_size);

    uv_mutex_unlock(&buffer->mutex_handle);
}

void cobra_buffer_clear(cobra_buffer_t *buffer) {
    uv_mutex_lock(&buffer->mutex_handle);
    cobra__buffer_clear(buffer);
    uv_mutex_unlock(&buffer->mutex_handle);
}

void cobra_buffer_fragment(cobra_buffer_t *buffer) {
    uv_mutex_lock(&buffer->mutex_handle);

    int length = cobra__buffer_length(buffer);
    if (length == 0 || buffer->read_pointer == buffer->head_pointer) {
        uv_mutex_unlock(&buffer->mutex_handle);
        return;
    }

    memmove(buffer->head_pointer, buffer->read_pointer, length);
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer + length;

    uv_mutex_unlock(&buffer->mutex_handle);
}

bool cobra_buffer_equals(cobra_buffer_t *buffer, const uint8_t *data, uint64_t length) {
    uv_mutex_lock(&buffer->mutex_handle);

    if (cobra_buffer_length(buffer) != length) {
        uv_mutex_unlock(&buffer->mutex_handle);
        return false;
    }

    bool result = memcmp(buffer->read_pointer, data, length) == 0;

    uv_mutex_unlock(&buffer->mutex_handle);
    return result;
}

void cobra__buffer_clear(cobra_buffer_t *buffer) {
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer;
}