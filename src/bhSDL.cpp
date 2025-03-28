#include <SDL3/SDL_video.h>
#include "bhSDL.hpp"
#include "bhConfig.hpp"

namespace bhSDL
{
  SDL_Window* g_mainWnd = nullptr;

  bool CreateWindow(const bhConfig& cfg)
  {
    SDL_PropertiesID props = bhConfig::CreateProperties(cfg);
    g_mainWnd = SDL_CreateWindowWithProperties(props);
    return (g_mainWnd != nullptr);
  }

  void DestroyWindow(SDL_Window*& wnd)
  {
    SDL_DestroyWindow(wnd);
    wnd = nullptr;
  }
}
