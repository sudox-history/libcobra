#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

void *cobra_queue_read(cobra_queue_t *queue) {
    uv_mutex_lock(&queue->mutex);

    if (cobra__queue_length(queue) == 0) {
        uv_mutex_unlock(&queue->mutex);
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
    if (cobra__queue_length(queue) == 0) {
        cobra__queue_clear(queue);
    }

    uv_mutex_unlock(&queue->mutex);
    return data;
}