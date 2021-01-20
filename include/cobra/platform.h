#ifndef COBRA_PLATFORM_H
#define COBRA_PLATFORM_H

#include <stdbool.h>
#ifdef COBRA_PLATFORM_PRIVATE
#include <stdint.h>
#endif  // COBRA_PLATFORM_PRIVATE

bool cobra_platform_is_big_endian();

#endif  // COBRA_PLATFORM_H
