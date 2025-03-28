#include <SDL.h>
#include "Software/bhPrimitivesSDL.h"
#include "Software/bhRaycaster.h"
#include "Math/bhMathUtil.h"
#include "Software/bhTexture.h"
#include "bhImage.h"

#define MAP_SZ 8
#define SHIFT_BITS 6

static int screenWidth = -1;
static int screenHeight = -1;

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    int x, y;
}
Vec2i;

typedef struct
{
    float x, y;
}
Vec2f;

typedef struct
{
    Vec2f point;
    float dist;
    short flags;
}
RayHit;

////////////////////////////////////////////////////////////////////////////////
int map[MAP_SZ][MAP_SZ] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
};

////////////////////////////////////////////////////////////////////////////////
int numRays = 0;
RayHit* rayHits = NULL;
Vec2f playerPos = { 1.5f, 1.5f };
float rotation = 0.f;
Vec2f playerFwd = { 0, 0 };

bhImage* wallTexture = NULL;

void CastRays();

void bhRaycaster_DrawMap(struct SDL_Surface* surf)
{
    const int sizeMod = 1 << SHIFT_BITS;

    CastRays();

    const float HEIGHT = 250;// screenHeight;
    int halfScreenHeight = screenHeight / 2;

    uint32_t color;
    for (int r = 0; r < numRays; ++r)
    {
        float mul = rayHits[r].dist / HEIGHT;
        float halfWallHeight = HEIGHT / (rayHits[r].dist * 2.f);

        float xc = rayHits[r].flags == 1 ? rayHits[r].point.x : rayHits[r].point.y;
        for (int y = 0; y < halfScreenHeight; ++y)
        {
            color = bhSampleTexture(wallTexture, xc, 0.5f + y * mul);
            bhPutPixel(surf, r, halfScreenHeight + y, color);
            //DrawPixel(r, halfScreenHeight + y, color);
            color = bhSampleTexture(wallTexture, xc, 0.5f - y * mul);
            bhPutPixel(surf, r, halfScreenHeight - y, color);
            //DrawPixel(r, halfScreenHeight - y, color);
            if (y >= (int)halfWallHeight)
            {
                break;
            }
        }
    }
}

void bhRaycaster_HandleInput()
{
    int numKeys = 0;
    const Uint8* keyState = SDL_GetKeyboardState(&numKeys);

    const int angleStep = 2;
    if (keyState[SDL_SCANCODE_RIGHT])
    {
        rotation -= angleStep;
    }
    if (keyState[SDL_SCANCODE_LEFT])
    {
        rotation += angleStep;
    }

    if (rotation < 0)
    {
        rotation += 360;
    }
    else if (rotation > 360)
    {
        rotation -= 360;
    }

    float rads = bhMath_Deg2Rad(-rotation);
    playerFwd.x = cosf(rads);
    playerFwd.y = sinf(rads);

    if (keyState[SDL_SCANCODE_UP])
    {
        playerPos.x += playerFwd.x * 0.05f;
        playerPos.y += playerFwd.y * 0.05f;
    }
    if (keyState[SDL_SCANCODE_DOWN])
    {
        playerPos.x -= playerFwd.x * 0.05f;
        playerPos.y -= playerFwd.y * 0.05f;
    }
    if (keyState[SDL_SCANCODE_TAB])
    {
        //showMap = showMap == 0 ? 1 : 0;
    }
}

void CastOnAxis(Vec2f ray, RayHit* rayHit)
{
    int hitXBlock = (int)playerPos.x;
    int hitYBlock = (int)playerPos.y;

    rayHit->dist = -1.f; // TODO

    // We are here because playerFwd.x == 0.f || playerFwd.y == 0.f
    if (ray.y == 0.f)
    {
        rayHit->flags = 0;
        rayHit->point.y = playerPos.y;
        if (ray.x > 0.f)
        {
            ++hitXBlock;
            for (; hitXBlock < MAP_SZ; ++hitXBlock)
            {
                if (map[hitYBlock][hitXBlock] > 0)
                {
                    rayHit->point.x = hitXBlock;
                    rayHit->dist = rayHit->point.x - playerPos.x;
                    return;
                }
            }
            rayHit->point.x = MAP_SZ;
        }
        else
        {
            --hitXBlock;
            for (; hitXBlock >= 0; --hitXBlock)
            {
                if (map[hitYBlock][hitXBlock] > 0)
                {
                    rayHit->point.x = hitXBlock + 1;
                    rayHit->dist = playerPos.x - rayHit->point.x;
                    return;
                }
            }
            rayHit->point.x = 0;
        }
    }
    else
    {
        // So here ray.x == 0.f
        rayHit->flags = 1;
        rayHit->point.x = playerPos.x;
        if (ray.y > 0.f)
        {
            ++hitYBlock;
            for (; hitYBlock < MAP_SZ; ++hitYBlock)
            {
                if (map[hitYBlock][hitXBlock] > 0)
                {
                    rayHit->point.y = hitYBlock;
                    rayHit->dist = rayHit->point.y - playerPos.y;
                    return;
                }
            }
            rayHit->point.y = MAP_SZ;
        }
        else
        {
            --hitYBlock;
            for (; hitYBlock >= 0; --hitYBlock)
            {
                if (map[hitYBlock][hitXBlock] > 0)
                {
                    rayHit->point.y = hitYBlock + 1;
                    rayHit->dist = playerPos.y - rayHit->point.y;
                    return;
                }
            }
            rayHit->point.y = 0;
        }
    }
}

void CastRays()
{
    const float aspect = 3.f / 4.f;
    Vec2f plane = { -playerFwd.y, playerFwd.x };
    for (int r = 0; r < numRays; ++r)
    {
        
        float screenX = 2.f * (float)r / (float)numRays - 1.f;
        screenX *= aspect;
        Vec2f ray = { playerFwd.x + plane.x * screenX, playerFwd.y + plane.y * screenX };

        if (ray.x == 0.f || ray.y == 0.f)
        {
            // Special ray treatment here
            CastOnAxis(ray, &(rayHits[r]));
            continue;
        }

        int hitXBlock = (int)playerPos.x;
        int hitYBlock = (int)playerPos.y;

        int checkX = (ray.x > 0.f) ? 1 : 0;
        checkX += hitXBlock;
        int checkY = (ray.y > 0.f) ? 1 : 0;
        checkY += hitYBlock;

        float distCheckX = ((float)checkX - playerPos.x) / ray.x;
        float distCheckY = ((float)checkY - playerPos.y) / ray.y;
        float distStepPerXUnit = fabsf(1 / ray.x);
        float distStepPerYUnit = fabsf(1 / ray.y);

        int xStep = (ray.x > 0.f) ? 1 : -1;
        int yStep = (ray.y > 0.f) ? 1 : -1;

        float minDist = -1.f;
        while (1)
        {
            if (distCheckX < distCheckY)
            {
                minDist = distCheckX;
                hitXBlock += xStep;
                distCheckX += distStepPerXUnit;
                rayHits[r].flags = 0;
            }
            else
            {
                minDist = distCheckY;
                hitYBlock += yStep;
                distCheckY += distStepPerYUnit;
                rayHits[r].flags = 1;
            }
            if (map[hitYBlock][hitXBlock] > 0)
            {
                break;
            }
        }

        float dx = minDist * ray.x;
        float dy = minDist * ray.y;
        rayHits[r].point.x = playerPos.x + dx;
        rayHits[r].point.y = playerPos.y + dy;
        rayHits[r].dist = dx * playerFwd.x + dy * playerFwd.y;
    }
}

int bhRaycaster_Init(int width, int height)
{
    screenWidth = width;
    screenHeight = height;

    numRays = screenWidth;
    rayHits = (RayHit*)malloc(numRays * sizeof(RayHit));
    
    wallTexture = bhImage_CreateFromFile("./Data/Textures/wall.png", 0);
    return wallTexture->retCode;
}

void bhRaycaster_Destroy()
{
    stbi_image_free(wallTexture->pixels);
    free(rayHits);
}
