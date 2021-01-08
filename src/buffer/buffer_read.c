#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

void cobra_buffer_read(cobra_buffer_t *buffer, uint8_t *data, uint64_t length) {
    uv_mutex_lock(&buffer->mutex_handle);
    cobra__buffer_read(buffer, data, length);
    uv_mutex_unlock(&buffer->mutex_handle);
}

uint64_t cobra_buffer_read_uint(cobra_buffer_t *buffer, int length) {
    uv_mutex_lock(&buffer->mutex_handle);

    if (length > sizeof(uint64_t))
        length = sizeof(uint64_t);

    if (cobra__buffer_length(buffer) < length) {
        uv_mutex_unlock(&buffer->mutex_handle);
        return 0;
    }

    uint8_t number_buffer[sizeof(uint64_t)] = {0};

    // Important notice:
    // We need to copy only lower digits of the number
    cobra__buffer_read(buffer, number_buffer + sizeof(uint64_t) - length, length);

    if (!cobra_platform_is_big_endian())
        cobra_utils_flip_array(number_buffer, sizeof(uint64_t));

    uv_mutex_unlock(&buffer->mutex_handle);
    return *(uint64_t *) number_buffer;
}

void cobra_buffer_read_void(cobra_buffer_t *buffer, uint64_t length) {
    uv_mutex_lock(&buffer->mutex_handle);

    if (cobra__buffer_length(buffer) < length) {
        uv_mutex_unlock(&buffer->mutex_handle);
        return;
    }

    buffer->read_pointer += length;
    if (buffer->read_pointer == buffer->write_pointer)
        cobra__buffer_clear(buffer);

    uv_mutex_unlock(&buffer->mutex_handle);
}

uint8_t *cobra_buffer_read_pointer(cobra_buffer_t *buffer) {
    uv_mutex_lock(&buffer->mutex_handle);
    uint8_t *read_pointer = buffer->read_pointer;
    uv_mutex_unlock(&buffer->mutex_handle);

    return read_pointer;
}

void cobra__buffer_read(cobra_buffer_t *buffer, uint8_t *data, uint64_t length) {
    if (cobra__buffer_length(buffer) < length)
        return;

    memcpy(data, buffer->read_pointer, length);
    buffer->read_pointer += length;

    if (buffer->read_pointer == buffer->write_pointer)
        cobra__buffer_clear(buffer);
}
