#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

void cobra_queue_clear(cobra_queue_t *queue) {
    uv_mutex_lock(&queue->mutex);
    cobra__queue_clear(queue);
    uv_mutex_unlock(&queue->mutex);
}

void cobra__queue_clear(cobra_queue_t *queue) {
    queue->read_pointer = queue->head_pointer;
    queue->write_pointer = queue->head_pointer;
}