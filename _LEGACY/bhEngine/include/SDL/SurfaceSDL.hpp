#ifndef SURFACE_SDL_HPP
#define SURFACE_SDL_HPP

#include <SDL3/SDL.h>

class SurfaceSDL
{
public:
  bool Create(int w, int h);
  bool Create(const char* filePath);
  bool Create(SDL_Window* wnd);
  virtual ~SurfaceSDL();
  void SetPixel(int x, int y, SDL_Color c);
  //const SDL_Surface* GetSDLSurface() const { return surface; }
  void BlitToSurface(SurfaceSDL& targetSurf);
  bool IsValid() const { return surface != nullptr; }

protected:
	__forceinline SDL_Surface* GetSurface() { return surface; }

private:
  SDL_Surface* surface{ nullptr };
  bool ownSurface{ true };
};

#endif //SURFACE_SDL_HPP
