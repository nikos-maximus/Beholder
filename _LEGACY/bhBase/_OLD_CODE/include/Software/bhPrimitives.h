#ifndef BH_PRIMITIVES_H
#define BH_PRIMITIVES_H

#include <inttypes.h>

__forceinline uint32_t bhColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = (r << 16) | (g << 8) | b;
    return color;
}

__forceinline uint32_t bhColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint32_t color = (a << 24) || (r << 16) | (g << 8) | b;
    return color;
}

#endif //BH_PRIMITIVES_H
