#include <malloc.h>
#include <string.h>
#include <uv.h>

#define COBRA_TRANSPORT_BUFFER_PRIVATE

#include "transport_buffer.h"

transport_buffer_t* transport_buffer_create(int initial_size) {
    transport_buffer_t* buffer = malloc(sizeof(transport_buffer_t));
    buffer->data = malloc(initial_size);
    buffer->length = initial_size;
    buffer->position = 0;

    return buffer;
}

void transport_buffer_insert(transport_buffer_t *buffer, void *data, int len) {
    int need_length = buffer->position + len;

    if (buffer->position + len < buffer->length) {
        int new_buffer_size = need_length * 2;
        unsigned char *new_buffer = malloc(new_buffer_size);
        memcpy(new_buffer, buffer->data, buffer->length);
        free(buffer->data);

        buffer->length = new_buffer_size;
        buffer->data = data;
    }

    memcpy(&buffer->data[buffer->position], data, len);
    buffer->position += len;
}

void transport_buffer_rewind(transport_buffer_t *buffer) {
    buffer->position = 0;
}

void transport_buffer_destroy(transport_buffer_t *buffer) {
    free(buffer->data);
    free(buffer);
}

int transport_buffer_read_int(transport_buffer_t *buffer, int count) {
    unsigned char *bytes = malloc(sizeof(int));
    memcpy(&bytes[sizeof(int) - count], &buffer->data[buffer->position], count);
    buffer->position += count;

    int number = (int) bytes;
    return ntohl(number);
}

int transport_buffer_position(transport_buffer_t *buffer) {
    return buffer->position;
}