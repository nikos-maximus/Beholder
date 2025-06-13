#include "bhMapRendererSDL3.hpp"
#include "bhMap.hpp"
#include "bhImage.hpp"
#include "bhPlatform.hpp"
#include "bhSDL3.hpp"

bhMapRendererSDL3::bhMapRendererSDL3(SDL_Renderer* _renderer)
  : bhMapRenderer()
  , renderer(_renderer)
{}

bool bhMapRendererSDL3::Init()
{
  //SDL_Surface* tileSurface = nullptr;

  tileSurfaces[TILE_WALL] = bhSDL3::CreateSurfaceFromImageFile("MapTileWall.png");
  tileSurfaces[TILE_FREE] = bhSDL3::CreateSurfaceFromImageFile("MapTileFree.png");

  SDL_Surface* textureSurf = nullptr;
  SDL_LockTextureToSurface(texture, nullptr, &textureSurf);
  SDL_Rect rect = { 0, 0, TILE_SZ_PX ,TILE_SZ_PX };
  for (uint8_t y = 0; y < ysiz; ++y)
  {
    for (uint8_t x = 0; x < xsiz; ++x)
    {
      SDL_BlitSurface(tileSurfaces[TILE_FREE], nullptr, textureSurf, &rect);
      rect.x += TILE_SZ_PX;
    }
    rect.x = 0;
    rect.y += TILE_SZ_PX;
  }
  SDL_UnlockTexture(texture);

  return true;
}

bool bhMapRendererSDL3::Reset(const bhMap* map)
{
  map->GetDims(xsiz, ysiz);

  Clear();
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, xsiz * TILE_SZ_PX, ysiz * TILE_SZ_PX);
  if (texture) return true;
  Clear();
  return false;
}

bhMapRendererSDL3::~bhMapRendererSDL3()
{
  Clear();
}

void bhMapRendererSDL3::DrawLayout(const bhMap* map)
{
  for (uint8_t y = 0; y < ysiz; ++y)
  {
    for (uint8_t x = 0; x < xsiz; ++x)
    {
      const bhMap::Block* block = map->GetBlock(x, y);
      if (block->Solid())
      {

      }
    }
  }
}

void bhMapRendererSDL3::Clear()
{
  for (uint8_t s = 0; s < NUM_TILE_IDX; ++s)
  {
    SDL_DestroySurface(tileSurfaces[s]);
  }
  if (texture)
  {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
}
