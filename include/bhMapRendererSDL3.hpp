#ifndef BH_WORLD_RENDERER_SDL3_HPP
#define BH_WORLD_RENDERER_SDL3_HPP

#include <SDL3/SDL.h> // TODO: Reduce this to necessary
#include "bhMapRenderer.hpp"

class bhMapRendererSDL3 : public bhMapRenderer
{
public:
  bhMapRendererSDL3() = delete;
  bhMapRendererSDL3(SDL_Renderer* _renderer);
  ~bhMapRendererSDL3();
  bool Reset(uint8_t _xsiz, uint8_t _ysiz);
  //void DrawGridFloor() const override;

protected:
private:
  void Clear();

  SDL_Renderer* renderer{ nullptr };
  SDL_Surface* surface{ nullptr };
  SDL_Texture* texture{ nullptr };
};

#endif //BH_WORLD_RENDERER_SDL3_HPP
