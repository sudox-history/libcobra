#define COBRA_QUEUE_PRIVATE
#include "cobra/queue.h"

void cobra_queue_clear(cobra_queue_t *queue) {
    queue->read_pointer = queue->head_pointer;
    queue->write_pointer = queue->head_pointer;
}