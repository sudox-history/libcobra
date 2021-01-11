#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

void cobra_queue_write(cobra_queue_t *queue, void *data) {
    if (cobra_queue_capacity(queue) == 0)
        return;

    *queue->write_pointer = data;

    queue->length++;
    queue->write_pointer++;
    if (queue->write_pointer - queue->head_pointer == queue->size)
        queue->write_pointer = queue->head_pointer;
}