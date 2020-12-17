#define COBRA_PLATFORM_PRIVATE
#include "cobra/platform.h"

bool cobra_platform_is_big_endian() {
    int tmp = 1;
    return *(uint8_t *) &tmp == 0;
}