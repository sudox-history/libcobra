#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

void cobra_queue_init(cobra_queue_t *queue, uint64_t size) {
    queue->size = size;
    queue->length = 0;
    queue->head_pointer = malloc(size * sizeof(void *));
    queue->read_pointer = queue->head_pointer;
    queue->write_pointer = queue->head_pointer;
}

void cobra_queue_deinit(cobra_queue_t *queue) {
    free(queue->head_pointer);
    free(queue);
}

uint64_t cobra_queue_length(cobra_queue_t *queue) {
    return queue->length;
}

uint64_t cobra_queue_capacity(cobra_queue_t *queue) {
    return queue->size - cobra_queue_length(queue);
}
