#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

void cobra_queue_lock(cobra_queue_t *queue) {
    uv_mutex_lock(&queue->mutex_handle);
}

void cobra_queue_unlock(cobra_queue_t *queue) {
    uv_mutex_unlock(&queue->mutex_handle);
}