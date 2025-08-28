#include "bhMapRenderer.hpp"
#include "bhMap.hpp"
#include "bhImage.hpp"
#include "bhPlatform.hpp"
#include "bhSDL3.hpp"

static const int TILE_SZ_PX{ 16 };

bhMapRenderer::bhMapRenderer(SDL_Renderer* _renderer)
  : renderer(_renderer)
{}

bool bhMapRenderer::Init()
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

bool bhMapRenderer::Reset(const bhMap* map)
{
  map->GetDims(xsiz, ysiz);

  Clear();
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, xsiz * TILE_SZ_PX, ysiz * TILE_SZ_PX);
  if (texture) return true;
  Clear();
  return false;
}

bhMapRenderer::~bhMapRenderer()
{
  Clear();
}

void bhMapRenderer::DrawLayout(const bhMap* map)
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

void bhMapRenderer::Clear()
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
