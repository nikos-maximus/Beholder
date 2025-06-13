#ifndef BH_SDL3_HPP
#define BH_SDL3_HPP

struct bhConfig;

struct SDL_Surface;

namespace bhSDL3
{
  SDL_Surface* CreateSurfaceFromImageFile(const char* fileName);
}

#endif //BH_SDL3_HPP
