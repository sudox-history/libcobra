#define COBRA_BUFFER_PRIVATE
#include "cobra/buffer.h"

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
