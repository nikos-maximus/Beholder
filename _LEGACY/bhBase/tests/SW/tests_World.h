#pragma once
#include "xp/bhRendererSW.h"

namespace tests_World
{
    bool Init(int w, int h, SDL_PixelFormat const* format);
    void Shutdown();
    SDL_Surface* GetRenderSurface();
    void Tick();
    void Render();
}
