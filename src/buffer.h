#ifndef COBRA_BUFFER_H
#define COBRA_BUFFER_H

#include <stdint.h>
#ifdef COBRA_BUFFER_PRIVATE
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#endif

typedef struct cobra_buffer_t cobra_buffer_t;

struct cobra_buffer_t {
    uint64_t size;
    uint8_t *head_pointer;
    uint8_t *read_pointer;
    uint8_t *write_pointer;
};

void cobra_buffer_init(cobra_buffer_t *buffer, uint64_t size);
void cobra_buffer_deinit(cobra_buffer_t *buffer);

uint64_t cobra_buffer_length(cobra_buffer_t *buffer);
uint64_t cobra_buffer_capacity(cobra_buffer_t *buffer);

void cobra_buffer_resize(cobra_buffer_t *buffer, uint64_t new_size);
void cobra_buffer_clear(cobra_buffer_t *buffer);
void cobra_buffer_fragment(cobra_buffer_t *buffer);

void cobra_buffer_write(cobra_buffer_t *buffer, uint8_t *data, uint64_t length);
void cobra_buffer_write_uint(cobra_buffer_t *buffer, uint64_t number, int length);
void cobra_buffer_write_void(cobra_buffer_t *buffer, uint64_t length);
uint8_t *cobra_buffer_write_pointer(cobra_buffer_t *buffer);

void cobra_buffer_read(cobra_buffer_t *buffer, uint8_t *data, uint64_t length);
uint64_t cobra_buffer_read_uint(cobra_buffer_t *buffer, int length);
void cobra_buffer_read_void(cobra_buffer_t *buffer, uint64_t length);
uint8_t *cobra_buffer_read_pointer(cobra_buffer_t *buffer);

#endif //COBRA_BUFFER_H
