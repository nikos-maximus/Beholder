#include "Software/bhPrimitivesSDL.h"
#include "bhUtil.h"
#include "Math/bhMathUtil.h"
#include "Software/bhPrimitives.h"

//bhColor_ui32 BH_BLACK_ui32  = 0xFF000000;
//bhColor_ui32 BH_WHITE_ui32  = 0xFFFFFFFF;
//bhColor_ui32 BH_RED_ui32    = 0xFFFF0000;

void bhPutPixel(const struct SDL_Surface* surf, int x, int y, Uint32 color)
{
    if (bhSurfaceCoordsAreValid(surf, x, y))
    {
        //const size_t pitch = surf->w * BYTES_PER_PIXEL;
        //Uint8* row = (Uint8*)(surf->pixels) + (y * pitch);
        Uint8* row = (Uint8*)(surf->pixels) + (y * surf->pitch); // TODO: We are assuming color depth == 32bpp
        Uint32* pixel = (Uint32*)row + x;
        *pixel = color;
    }
}

void bhLine(int startX, int startY, int endX, int endY)
{
    if (bhMath_Abs(endX - startX) > bhMath_Abs(endY - startY))
    {
        if (startX > endX)
        {
            bhUtil_Swap(&startX, &endX);
            bhUtil_Swap(&startY, &endY);
        }
        float yDiff = (float)(endY - startY) / (float)(endX - startX);
        float y = (float)startY;
        for (int x = startX; x <= endX; ++x)
        {
            //DrawPixel(x, (int)y, BLACK);
            y += yDiff;
        }
    }
    else
    {
        if (startY > endY)
        {
            bhUtil_Swap(&startX, &endX);
            bhUtil_Swap(&startY, &endY);
        }
        float xDiff = (float)(endX - startX) / (float)(endY - startY);
        float x = (float)startX;
        for (int y = startY; y <= endY; ++y)
        {
            //DrawPixel((int)x, y, BLACK);
            x += xDiff;
        }
    }
}

void bhPaintTestGradient(SDL_Surface* surf, int xOffset, int yOffset)
{
    Uint8* row = (Uint8*)(surf->pixels);
    for (int y = 0; y < surf->h; ++y)
    {
        Uint32* pixel = (Uint32*)row; // TODO: We are assuming color depth == 32bpp
        for (int x = 0; x < surf->w; ++x)
        {
            *pixel = bhColor3ub(0, y + yOffset, x + xOffset);
            ++pixel;
        }
        row += surf->pitch;
    }
}
