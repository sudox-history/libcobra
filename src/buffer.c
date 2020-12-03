#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

void cobra_buffer_init(cobra_buffer_t *buffer, uint64_t size) {
    buffer->size = size;
    buffer->head_pointer = malloc(size);
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer;
}

void cobra_buffer_deinit(cobra_buffer_t *buffer) {
    free(buffer->head_pointer);
}

uint64_t cobra_buffer_length(cobra_buffer_t *buffer) {
    return (uint64_t) (buffer->write_pointer - buffer->read_pointer);
}

uint64_t cobra_buffer_capacity(cobra_buffer_t *buffer) {
    return buffer->size - (uint64_t) (buffer->write_pointer - buffer->head_pointer);
}

void cobra_buffer_resize(cobra_buffer_t *buffer, uint64_t new_size) {
    if (buffer->size >= new_size)
        return;

    buffer->size = new_size;
    buffer->head_pointer = realloc(buffer->head_pointer, new_size);
}

void cobra_buffer_clear(cobra_buffer_t *buffer) {
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer;
}

void cobra_buffer_fragment(cobra_buffer_t *buffer) {
    int length = cobra_buffer_length(buffer);

    if (length == 0 || buffer->read_pointer == buffer->head_pointer)
        return;

    memmove(buffer->head_pointer, buffer->read_pointer, length);
    buffer->read_pointer = buffer->head_pointer;
    buffer->write_pointer = buffer->head_pointer + length;
}

void cobra_buffer_write(cobra_buffer_t *buffer, uint8_t *data, uint64_t length) {
    if (cobra_buffer_capacity(buffer) < length)
        return;

    memcpy(buffer->write_pointer, data, length);
    buffer->write_pointer += length;
}

void cobra_buffer_write_uint(cobra_buffer_t *buffer, uint64_t number, int length) {
    if (length > sizeof(uint64_t))
        length = sizeof(uint64_t);

    if (cobra_buffer_capacity(buffer) < length)
        return;

    uint8_t *number_buffer = (uint8_t *) &number;
    if (!cobra_platform_is_big_endian())
        cobra_utils_flip_array(number_buffer, sizeof(uint64_t));

    // Important notice:
    // We need to copy only lower digits of the number
    cobra_buffer_write(buffer, number_buffer + sizeof(uint64_t) - length, length);
}

void cobra_buffer_write_void(cobra_buffer_t *buffer, uint64_t length) {
    if (cobra_buffer_capacity(buffer) < length)
        return;

    buffer->write_pointer += length;
}

uint8_t *cobra_buffer_write_pointer(cobra_buffer_t *buffer) {
    return buffer->write_pointer;
}

void cobra_buffer_read(cobra_buffer_t *buffer, uint8_t *data, uint64_t length) {
    if (cobra_buffer_length(buffer) < length)
        return;

    memcpy(data, buffer->read_pointer, length);
    buffer->read_pointer += length;

    if (buffer->read_pointer == buffer->write_pointer)
        cobra_buffer_clear(buffer);
}

uint64_t cobra_buffer_read_uint(cobra_buffer_t *buffer, int length) {
    if (length > sizeof(uint64_t))
        length = sizeof(uint64_t);

    if (cobra_buffer_length(buffer) < length)
        return 0;

    uint8_t number_buffer[sizeof(uint64_t)] = {0};

    // Important notice:
    // We need to copy only lower digits of the number
    cobra_buffer_read(buffer, number_buffer + sizeof(uint64_t) - length, length);

    if (!cobra_platform_is_big_endian())
        cobra_utils_flip_array(number_buffer, sizeof(uint64_t));

    return *(uint64_t *) number_buffer;
}

void cobra_buffer_read_void(cobra_buffer_t *buffer, uint64_t length) {
    if (cobra_buffer_length(buffer) < length)
        return;

    buffer->read_pointer += length;
    if (buffer->read_pointer == buffer->write_pointer)
        cobra_buffer_clear(buffer);
}

uint8_t *cobra_buffer_read_pointer(cobra_buffer_t *buffer) {
    return buffer->read_pointer;
}

bool cobra_buffer_equals(cobra_buffer_t *buffer, const uint8_t *data, uint64_t length) {
    if (cobra_buffer_length(buffer) != length)
        return false;

    for (int i = 0; i < length; i++) {
        if (buffer->read_pointer[i] != data[i])
            return false;
    }

    return true;
}