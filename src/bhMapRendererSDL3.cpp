#include "bhMapRendererSDL3.hpp"

bhMapRendererSDL3::bhMapRendererSDL3(SDL_Renderer* _renderer)
  : bhMapRenderer()
  , renderer(_renderer)
{}

bool bhMapRendererSDL3::Reset(uint8_t _xsiz, uint8_t _ysiz)
{
  Clear();
  (surface = SDL_CreateSurface(_xsiz, _ysiz, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA8888)) &&
    (texture = SDL_CreateTextureFromSurface(renderer, surface));
  return (surface != nullptr) && (texture != nullptr);
}

bhMapRendererSDL3::~bhMapRendererSDL3()
{
  Clear();
}

//void bhMapRendererSDL3::DrawGridFloor() const
//{}

void bhMapRendererSDL3::Clear()
{
  if (texture) SDL_DestroyTexture(texture);
  if (surface) SDL_DestroySurface(surface);
}
