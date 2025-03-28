#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include "bhConfig.hpp"

SDL_Window* g_mainWnd = nullptr;

inline Uint32 bhMapRGBAUint8(SDL_Surface* surf, Uint8 r, Uint8 g, Uint8 b, Uint8 a = SDL_ALPHA_OPAQUE)
{
  const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surf->format);
  return SDL_MapRGBA(details, nullptr, r, g, b, a);
}

inline Uint32 bhMapRGBAFloat(SDL_Surface* surf, float r, float g, float b, float a = 1.0f)
{
  const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surf->format);
  return SDL_MapRGBA(details, nullptr, Uint8(255.999f * r), Uint8(255.999f * g), Uint8(255.999f * b), Uint8(255.999f * a));
}

int main(int argc, char* argv[])
{
  bool running = true;

  if (SDL_Init(SDL_INIT_VIDEO))
  {
    bhConfig cfg;
    //bhConfig::Load()
    SDL_PropertiesID props = bhConfig::CreateProperties(cfg);

    g_mainWnd = SDL_CreateWindowWithProperties(props);
    if (g_mainWnd)
    {
      SDL_Surface* wndSurface = SDL_GetWindowSurface(g_mainWnd);

      SDL_Surface* compoSurface = SDL_CreateSurface(wndSurface->w, wndSurface->h, SDL_PIXELFORMAT_RGBA8888);
      Uint32* pixels = reinterpret_cast<Uint32*>(compoSurface->pixels);
      const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(compoSurface->format);
      for (int y = 0; y < compoSurface->h; ++y)
      {
        for (int x = 0; x < compoSurface->w; ++x)
        {
          *pixels++ = SDL_MapRGBA(details, nullptr, Uint8(float(x) / float(compoSurface->w) * 255.999f), Uint8(float(y) / float(compoSurface->h) * 255.999f), 0, SDL_ALPHA_OPAQUE);
        }
      }
      SDL_BlitSurface(compoSurface, nullptr, wndSurface, nullptr);
      SDL_DestroySurface(compoSurface);

      SDL_ShowWindow(g_mainWnd); // TODO: Error check
      while (running)
      {
        SDL_Event evt;
        while (SDL_PollEvent(&evt))
        {
          switch (evt.type)
          {
            case SDL_EVENT_QUIT:
            {
              running = false;
              break;
            }
          }
        }
        SDL_UpdateWindowSurface(g_mainWnd);
      }
      SDL_DestroyWindow(g_mainWnd);
    }
    SDL_Quit();
    return 0;
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
}
