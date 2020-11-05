#ifndef COBRA_BUFFER_H
#define COBRA_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct cobra_buffer_t cobra_buffer_t;

#ifdef COBRA_BUFFER_PRIVATE

struct cobra_buffer_t {
    uint8_t *data;
    int len;

    int read_pos;
    int write_pos;
};

#endif

cobra_buffer_t *cobra_buffer_create(int len);
void cobra_buffer_destroy(cobra_buffer_t *buffer);

void cobra_buffer_write(cobra_buffer_t *buffer, uint8_t *data, int len);
void cobra_buffer_write_pointer(cobra_buffer_t *buffer, uint8_t **data, int *cap);
void cobra_buffer_write_uint16(cobra_buffer_t *buffer, uint16_t number);

void cobra_buffer_read(cobra_buffer_t *buffer, uint8_t *data, int len);
uint16_t cobra_buffer_read_uint16(cobra_buffer_t *buffer);

void cobra_buffer_resize(cobra_buffer_t *buffer, int new_len);
void cobra_buffer_clear(cobra_buffer_t *buffer);

#endif //COBRA_BUFFER_H