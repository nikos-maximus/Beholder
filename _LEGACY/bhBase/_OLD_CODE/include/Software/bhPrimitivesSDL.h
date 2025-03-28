#ifndef BH_PRIMITIVES_SDL_H
#define BH_PRIMITIVES_SDL_H

#define BYTES_PER_PIXEL 4

#include <SDL_surface.h>

//extern bhColor_ui32 BH_BLACK_ui32;
//extern bhColor_ui32 BH_WHITE_ui32;
//extern bhColor_ui32 BH_RED_ui32;

__forceinline int bhSurfaceCoordsAreValid(const struct SDL_Surface* surf, int x, int y)
{
    return ((0 <= x) && (x < surf->w) && (0 <= y) && (y < surf->h));
}

void bhPutPixel(const struct SDL_Surface* surf, int x, int y, Uint32 color);
void bhPaintTestGradient(SDL_Surface* surf, int xOffset, int yOffset);

#endif //BH_PRIMITIVES_SDL_H
