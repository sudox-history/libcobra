#include <stdio.h>
#include <stdlib.h>
#include "internal/queue.h"

int main() {
    cobra_queue_t *queue = malloc(sizeof(cobra_queue_t));
    cobra_queue_init(queue, 5);

    cobra_queue_push(queue, (void *) 14);
    cobra_queue_push(queue, (void *) 15);
    cobra_queue_push(queue, (void *) 16);
    cobra_queue_push(queue, (void *) 17);
    cobra_queue_push(queue, (void *) 18);
    cobra_queue_shift(queue);
    cobra_queue_push(queue, (void *) 19);

    printf("%d\n", cobra_queue_shift(queue));
    printf("%d\n", cobra_queue_shift(queue));
    printf("%d\n", cobra_queue_shift(queue));
    printf("%d\n", cobra_queue_shift(queue));
    printf("%d\n", cobra_queue_shift(queue));
    cobra_queue_deinit(queue);
}