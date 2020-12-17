#define COBRA_UTILS_PRIVATE
#include "cobra/utils.h"

/**
 * Flips the array
 *
 * @param array - Pointer to array
 * @param length - Array length
 */
void cobra_utils_flip_array(uint8_t* array, int length) {
    for (int i = 0; i < length / 2; i++) {
        int temp = array[i];
        array[i] = array[length - i - 1];
        array[length - i - 1] = temp;
    }
}

/**
 * Prints an array in human readable form
 *
 * @param array - Pointer to array
 * @param length - Array length
 */
void cobra_utils_print_array(uint8_t* array, int length) {
    for (int i = 0; i < length; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}