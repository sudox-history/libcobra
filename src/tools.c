#include "tools.h"

bool cobra_tools_is_platform_big_endian() {
    int number = 1;
    return *(uint8_t *) &number == 0;
}

void cobra_tools_array_reverse(uint8_t *data, int length) {
    for (int i = 0; i < length / 2; i++) {
        int temp = data[i];
        data[i] = data[length - i - 1];
        data[length - i - 1] = temp;
    }
}

void cobra_tools_array_print(uint8_t *data, int length) {
    for (int i = 0; i < length; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
}