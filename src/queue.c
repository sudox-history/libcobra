#define COBRA_QUEUE_PRIVATE
#include "queue.h"

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

void cobra_queue_push(cobra_queue_t *queue, void *data) {
    if (cobra_queue_capacity(queue) == 0) {
        return;
    }

    // Saving data
    *queue->write_pointer = data;

    queue->length++;
    queue->write_pointer++;
    if (queue->write_pointer - queue->head_pointer == queue->size) {
        queue->write_pointer = queue->head_pointer;
    }
}

void *cobra_queue_shift(cobra_queue_t *queue) {
    if (cobra_queue_length(queue) == 0) {
        return NULL;
    }

    // Getting data
    void *data = *queue->read_pointer;

    queue->length--;
    queue->read_pointer++;
    if (queue->read_pointer - queue->head_pointer == queue->size) {
        queue->read_pointer = queue->head_pointer;
    }

    // One more check for length
    if (cobra_queue_length(queue) == 0) {
        cobra_queue_clear(queue);
    }

    return data;
}

void cobra_queue_clear(cobra_queue_t *queue) {
    queue->read_pointer = queue->head_pointer;
    queue->write_pointer = queue->head_pointer;
}