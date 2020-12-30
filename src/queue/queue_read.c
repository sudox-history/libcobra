#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

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