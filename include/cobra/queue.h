#ifndef COBRA_QUEUE_H
#define COBRA_QUEUE_H

#include <stdbool.h>
#include <stdint.h>
#include <uv.h>
#ifdef COBRA_QUEUE_PRIVATE
#include <stddef.h>
#include <stdlib.h>
#endif

typedef struct cobra_queue_t cobra_queue_t;

struct cobra_queue_t {
    uv_mutex_t mutex_handle;
    uint64_t   size;
    uint64_t   length;
    void       **head_pointer;
    void       **read_pointer;
    void       **write_pointer;
};

/**
 * Base methods
 */
void cobra_queue_init(cobra_queue_t *queue, uint64_t size);
void cobra_queue_deinit(cobra_queue_t *queue);
uint64_t cobra_queue_length(cobra_queue_t *queue);
uint64_t cobra_queue_capacity(cobra_queue_t *queue);
#ifdef COBRA_QUEUE_PRIVATE
uint64_t cobra__queue_length(cobra_queue_t *queue);
uint64_t cobra__queue_capacity(cobra_queue_t *queue);
#endif

/**
 * Memory methods
 */
void cobra_queue_clear(cobra_queue_t *queue);
#ifdef COBRA_QUEUE_PRIVATE
void cobra__queue_clear(cobra_queue_t *queue);
#endif

/**
 * Write methods
 */
void cobra_queue_write(cobra_queue_t *queue, void *data);
// TODO: Implement cobra_queue_write_start

/**
 * Read methods
 */
void *cobra_queue_read(cobra_queue_t *queue);
// TODO: Implement cobra_queue_read_start


#endif//COBRA_QUEUE_H
