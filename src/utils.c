#define COBRA_UTILS_PRIVATE
#include "utils.h"

void cobra_utils_flip_array(uint8_t *array, int length) {
    for (int i = 0; i < length / 2; i++) {
        int temp = array[i];
        array[i] = array[length - i - 1];
        array[length - i - 1] = temp;
    }
}

void cobra_utils_print_array(uint8_t *array, int length) {
    for (int i = 0; i < length; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}