#ifndef BH_WORLD_RENDERER_SDL3_HPP
#define BH_WORLD_RENDERER_SDL3_HPP

#include <SDL3/SDL.h> // TODO: Reduce this to necessary
#include "bhMapRenderer.hpp"

class bhMap;

class bhMapRendererSDL3 : public bhMapRenderer
{
public:
  bhMapRendererSDL3() = delete;
  bhMapRendererSDL3(SDL_Renderer* _renderer);
  ~bhMapRendererSDL3();

  bool Init() override final;
  bool Reset(const bhMap* map);
  //void DrawGridFloor() const override;

  void DrawLayout(const bhMap* map);

  const SDL_Texture* GetTexture() const { return texture; }

protected:
private:
  void Clear();

  SDL_Renderer* renderer{ nullptr };
  //SDL_Surface* surface{ nullptr };
  SDL_Texture* texture{ nullptr };

  enum TileIdx
  {
    TILE_WALL,
    TILE_FREE,
    
    NUM_TILE_IDX
  };

  SDL_Surface* tileSurfaces[NUM_TILE_IDX] = {};

  uint8_t xsiz{ 0 }, ysiz{ 0 };
};

#endif //BH_WORLD_RENDERER_SDL3_HPP
