#include <stdio.h>

#include "buffer.h"

int main() {
    cobra_buffer_t buffer1;
    cobra_buffer_init(&buffer1, 10);

    cobra_buffer_t buffer2;
    cobra_buffer_init(&buffer2, 10);

    uint8_t data[3] = {1, 2, 3};
    uint8_t data2[4] = {1, 2, 3, 4};
    cobra_buffer_write(&buffer1, data, 3);
    cobra_buffer_write(&buffer2, data2, 4);

    printf("%d\n", cobra_buffer_equals(&buffer1, &buffer2));
}