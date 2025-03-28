#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "SDL/SurfaceSDL.hpp"
#include "bhLog.h"

bool SurfaceSDL::Create(int w, int h)
{
  surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
  if (!surface)
  {
    bhLog_Message(bhLogPriority::LP_ERROR, SDL_GetError());
    return false;
  }
  return true;
}

bool SurfaceSDL::Create(const char* filePath)
{
  static constexpr auto IMG_FMT = STBI_rgb_alpha;

  int w, h, comp;
  stbi_uc* pixels = stbi_load(filePath, &w, &h, &comp, IMG_FMT);
  if (pixels)
  {
    surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
    memcpy(surface->pixels, pixels, w * h * IMG_FMT);
    stbi_image_free(pixels);
    if (surface)
    {
      return true;
    }
    else
    {
      bhLog_Message(bhLogPriority::LP_ERROR, SDL_GetError());
    }
  }
  return false;
}

bool SurfaceSDL::Create(SDL_Window* wnd)
{
  ownSurface = false;
  surface = SDL_GetWindowSurface(wnd);
  if (!surface)
  {
    bhLog_Message(bhLogPriority::LP_ERROR, SDL_GetError());
    return false;
  }
  return true;
}

SurfaceSDL::~SurfaceSDL()
{
  if (ownSurface)
  {
    SDL_DestroySurface(surface);
    surface = nullptr;
  }
}

void SurfaceSDL::SetPixel(int x, int y, SDL_Color c)
{
  SDL_Color* pixels = static_cast<SDL_Color*>(surface->pixels);
  pixels[y * surface->w + x] = c;
}

void SurfaceSDL::BlitToSurface(SurfaceSDL& targetSurf)
{
  SDL_BlitSurface(surface, nullptr, targetSurf.surface, nullptr);
}
