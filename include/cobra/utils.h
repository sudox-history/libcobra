

#ifndef COBRA_UTILS_H
#define COBRA_UTILS_H

#include <stdint.h>
#ifdef COBRA_UTILS_PRIVATE
#include <stdio.h>
#endif //COBRA_UTILS_PRIVATE

/**
 * Flips the array
 *
 * @param array - Pointer to array
 * @param length - Array length
 */
void cobra_utils_flip_array(uint8_t *array, int length);

/**
 * Prints an array in human readable form
 *
 * @param array - Pointer to array
 * @param length - Array length
 */
void cobra_utils_print_array(uint8_t *array, int length);

#endif //COBRA_UTILS_H
