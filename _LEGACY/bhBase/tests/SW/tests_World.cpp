#include "tests_World.h"
#include "Math/bhMath.h"
#include "bhImage.h"
#include <memory.h>
#include <stdlib.h>

#define PALETTE_SIZE 32

namespace tests_World
{
    SDL_Surface* g_worldSurface = nullptr;
    SDL_Surface* test = nullptr;
    int g_width = 0, g_height = 0;
    SDL_Color g_palette[PALETTE_SIZE];
    int* g_fireEnergy = nullptr;

    void InitEnergies()
    {
        memset(g_palette, 0, sizeof(SDL_Color) * PALETTE_SIZE);
        int increment = 256 / PALETTE_SIZE;
        int value = 0;
        for (int c = 0; c < PALETTE_SIZE; ++c)
        {
            g_palette[c].r = value;
            value += increment;
        }

        g_fireEnergy = static_cast<int*>(calloc(g_height * g_width, sizeof(int*)));

        int rowOffset = (g_height - 1) * g_width;
        for (int col = 0; col < g_width; ++col)
        {
            g_fireEnergy[rowOffset + col] = PALETTE_SIZE - 1;
        }
    }

    void DestroyEnergies()
    {
        free(g_fireEnergy);
        g_fireEnergy = nullptr;
    }

    bool Init(int w, int h, SDL_PixelFormat const* format)
    {
        g_worldSurface = bhRendererSW::CreateSurface(w, h, format);
        if (g_worldSurface != nullptr)
        {
            g_width = w;
            g_height = h;
            InitEnergies();
            bhResourceIndex_t newImageIndex = bhImage::CreateFromFile("HelenMirren32.png");
            test = bhRendererSW::CreateSurfaceFromImage(bhImage::Get(newImageIndex), bhRendererSW::GetWindowFormat());
            return true;
        }
        return false;
    }

    void Shutdown()
    {
        DestroyEnergies();
        bhRendererSW::DestroySurface(&test);
        bhRendererSW::DestroySurface(&g_worldSurface);
    }

    SDL_Surface* GetRenderSurface()
    {
        return g_worldSurface;
    }

    void Tick()
    {
        int rowOffset = (g_height - 2) * g_width;
        for (int row = g_height - 2; row > 0; --row)
        {
            for (int col = 0; col < g_width; ++col)
            {
                //g_fireEnergy[row][col] = g_fireEnergy[row + 1][(col + 1) % g_width] - rand() % 2;
                g_fireEnergy[rowOffset + col] = bhMath::Max(0, g_fireEnergy[rowOffset + g_width + col] - rand() % 3);
            }
            rowOffset -= g_width;
        }
    }

    void Render()
    {
        SDL_BlitSurface(test, nullptr, g_worldSurface, nullptr);
        //if (bhRendererSW::LockSurface(g_worldSurface))
        //{
        //    int rowOffset = 0;
        //    for (int row = 0; row < g_height; ++row)
        //    {
        //        for (int col = 0; col < g_width; ++col)
        //        {
        //            bhRendererSW::WritePixel(g_worldSurface, col, row, &(g_palette[g_fireEnergy[rowOffset + col]]));
        //        }
        //        rowOffset += g_width;
        //    }
        //    bhRendererSW::UnlockSurface(g_worldSurface);
        //}
    }
}
