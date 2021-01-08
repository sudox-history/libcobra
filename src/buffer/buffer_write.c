#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

void cobra_buffer_write(cobra_buffer_t *buffer, uint8_t *data, uint64_t length) {
    uv_mutex_lock(&buffer->mutex_handle);
    cobra__buffer_write(buffer, data, length);
    uv_mutex_unlock(&buffer->mutex_handle);
}

void cobra_buffer_write_uint(cobra_buffer_t *buffer, uint64_t number, int length) {
    uv_mutex_lock(&buffer->mutex_handle);

    if (length > sizeof(uint64_t))
        length = sizeof(uint64_t);

    if (cobra_buffer_capacity(buffer) < length) {
        uv_mutex_unlock(&buffer->mutex_handle);
        return;
    }

    uint8_t *number_buffer = (uint8_t *) &number;
    if (!cobra_platform_is_big_endian())
        cobra_utils_flip_array(number_buffer, sizeof(uint64_t));

    // Important notice:
    // We need to copy only lower digits of the number
    cobra__buffer_write(buffer, number_buffer + sizeof(uint64_t) - length, length);

    uv_mutex_unlock(&buffer->mutex_handle);
}

void cobra_buffer_write_void(cobra_buffer_t *buffer, uint64_t length) {
    uv_mutex_lock(&buffer->mutex_handle);

    if (cobra_buffer_capacity(buffer) < length) {
        uv_mutex_unlock(&buffer->mutex_handle);
        return;
    }

    buffer->write_pointer += length;
    uv_mutex_unlock(&buffer->mutex_handle);
}

uint8_t *cobra_buffer_write_pointer(cobra_buffer_t *buffer) {
    uv_mutex_lock(&buffer->mutex_handle);
    uint8_t *write_pointer = buffer->write_pointer;
    uv_mutex_unlock(&buffer->mutex_handle);

    return write_pointer;
}

void cobra__buffer_write(cobra_buffer_t *buffer, uint8_t *data, uint64_t length) {
    if (cobra_buffer_capacity(buffer) < length)
        return;

    memcpy(buffer->write_pointer, data, length);
    buffer->write_pointer += length;
}