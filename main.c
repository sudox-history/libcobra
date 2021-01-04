#include <stdio.h>
#include <stdlib.h>
#include "cobra/queue.h"

int main() {
    cobra_queue_t queue;
    cobra_queue_init(&queue, 10);

    cobra_queue_write(&queue, (void *) 1);
    cobra_queue_write(&queue, (void *) 2);
    cobra_queue_write(&queue, (void *) 3);

    printf("%lu\n", (uint64_t) cobra_queue_read(&queue));
    cobra_queue_write(&queue, (void *) 4);

    printf("%lu\n", (uint64_t) cobra_queue_read(&queue));
    printf("%lu\n", (uint64_t) cobra_queue_read(&queue));
    printf("%lu\n", (uint64_t) cobra_queue_read(&queue));
    cobra_queue_deinit(&queue);
}