#ifndef COBRA_TOOLS_H
#define COBRA_TOOLS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

bool cobra_tools_is_platform_big_endian();

void cobra_tools_array_reverse(uint8_t *data, int length);
void cobra_tools_array_print(uint8_t *data, int length);

#endif //COBRA_TOOLS_H
