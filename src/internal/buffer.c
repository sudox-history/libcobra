#include <stdlib.h>
#include <string.h>

#define COBRA_BUFFER_PRIVATE
#include "buffer.h"

cobra_buffer_t *cobra_buffer_create(int len) {
    cobra_buffer_t *buffer = malloc(sizeof(cobra_buffer_t));
    buffer->data = malloc(len);
    buffer->len = len;
    buffer->read_pos = 0;
    buffer->write_pos = 0;
    return buffer;
}

void cobra_buffer_destroy(cobra_buffer_t *buffer) {
    free(buffer->data);
    free(buffer);
}

void cobra_buffer_resize(cobra_buffer_t *buffer, int new_len) {
    if (buffer->len >= new_len)
        return;

    buffer->data = realloc(buffer->data, new_len);
    buffer->len = new_len;
}

int cobra_buffer_len(cobra_buffer_t *buffer) {
    return buffer->write_pos - buffer->read_pos;
}

void cobra_buffer_skip(cobra_buffer_t *buffer, int len) {
    if (buffer->write_pos + len > buffer->len)
        return;

    buffer->write_pos += len;
}

void cobra_buffer_write(cobra_buffer_t *buffer, uint8_t *data, int len) {
    if (buffer->write_pos + len > buffer->len)
        return;

    memcpy(buffer->data + buffer->write_pos, data, len);
    buffer->write_pos += len;
}

int cobra_buffer_write_pointer(cobra_buffer_t *buffer, uint8_t **data) {
    *data = buffer->data + buffer->write_pos;
    return buffer->len - buffer->write_pos;
}

bool buffer_is_platform_big_endian() {
    int number = 1;
    return *(char *) &number == 0;
}

void cobra_buffer_write_uint16(cobra_buffer_t *buffer, uint16_t number) {
    if (buffer->write_pos + sizeof(uint16_t) > buffer->len)
        return;

    if (!buffer_is_platform_big_endian())
        number = number >> 8 | number << 8; // NOLINT(hicpp-signed-bitwise)

    cobra_buffer_write(buffer, (uint8_t *) &number, sizeof(uint16_t));
}

void cobra_buffer_read(cobra_buffer_t *buffer, uint8_t *data, int len) {
    if (buffer->write_pos - buffer->read_pos < len)
        return;

    memcpy(data, buffer->data + buffer->read_pos, len);
    buffer->read_pos += len;

    if (buffer->read_pos == buffer->write_pos)
        cobra_buffer_clear(buffer);
}

uint16_t cobra_buffer_read_uint16(cobra_buffer_t *buffer) {
    if (buffer->write_pos - buffer->read_pos < sizeof(uint16_t))
        return 0;

    uint8_t result_buffer[sizeof(uint16_t)];
    cobra_buffer_read(buffer, result_buffer, sizeof(uint16_t));

    uint16_t result = *(uint16_t *) result_buffer;
    if (!buffer_is_platform_big_endian())
        result = result >> 8 | result << 8; // NOLINT(hicpp-signed-bitwise)

    return result;
}

bool cobra_buffer_equals(cobra_buffer_t *buffer, const uint8_t *data, int len) {
    if (cobra_buffer_len(buffer) != len)
        return false;

    for (int i = 0; i < len; i++) {
        if (data[i] != buffer->data[buffer->read_pos + i])
            return false;
    }

    return true;
}

void cobra_buffer_clear(cobra_buffer_t *buffer) {
    buffer->read_pos = buffer->write_pos = 0;
}

void cobra_buffer_fragment(cobra_buffer_t *buffer) {
    int len = cobra_buffer_len(buffer);

    if (len == 0 || buffer->read_pos == 0)
        return;

    memcpy(buffer->data, buffer->data + buffer->read_pos, len);
    buffer->read_pos = 0;
    buffer->write_pos = len;
}