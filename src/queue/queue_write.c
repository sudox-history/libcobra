#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

void cobra_queue_write(cobra_queue_t *queue, void *data) {
    uv_mutex_lock(&queue->mutex_handle);

    if (cobra__queue_capacity(queue) == 0) {
        uv_mutex_unlock(&queue->mutex_handle);
        return;
    }

    // Saving data
    *queue->write_pointer = data;

    queue->length++;
    queue->write_pointer++;
    if (queue->write_pointer - queue->head_pointer == queue->size)
        queue->write_pointer = queue->head_pointer;

    uv_mutex_unlock(&queue->mutex_handle);
}