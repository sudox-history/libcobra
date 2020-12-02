#ifndef COBRA_QUEUE_H
#define COBRA_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#ifdef COBRA_QUEUE_PRIVATE
#include <stddef.h>
#include <stdlib.h>
#endif

typedef struct cobra_queue_t cobra_queue_t;

struct cobra_queue_t {
    uint64_t size;
    uint64_t length;
    void **head_pointer;
    void **read_pointer;
    void **write_pointer;
};

void cobra_queue_init(cobra_queue_t *queue, uint64_t size);
void cobra_queue_deinit(cobra_queue_t *queue);

uint64_t cobra_queue_length(cobra_queue_t *queue);
uint64_t cobra_queue_capacity(cobra_queue_t *queue);

void cobra_queue_push(cobra_queue_t *queue, void *data);
void *cobra_queue_shift(cobra_queue_t *queue);

void cobra_queue_clear(cobra_queue_t *queue);

#endif //COBRA_QUEUE_H
