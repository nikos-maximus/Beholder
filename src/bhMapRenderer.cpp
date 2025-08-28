#include <backends/imgui_impl_sdl3.h>
//#include <backends/imgui_impl_sdlrenderer3.h>
#include "bhMapRenderer.hpp"
#include "bhMap.hpp"
#include "bhImage.hpp"
#include "bhPlatform.hpp"
#include "bhSDL3.hpp"
#include "bhMathUtil.hpp"

static const int TILE_SZ_PX{ 16 };

bhMapRenderer::bhMapRenderer(SDL_Renderer* _renderer)
  : renderer(_renderer)
{}

bool bhMapRenderer::Init()
{
  tileSurfaces[TILE_WALL] = bhSDL3::CreateSurfaceFromImageFile("MapTileWall.png");
  tileSurfaces[TILE_FREE] = bhSDL3::CreateSurfaceFromImageFile("MapTileFree.png");

  SDL_Surface* textureSurf = nullptr;
  SDL_LockTextureToSurface(texture, nullptr, &textureSurf);
  SDL_Rect rect = { 0, 0, TILE_SZ_PX ,TILE_SZ_PX };
  for (uint8_t y = 0; y < ysiz; ++y)
  {
    for (uint8_t x = 0; x < xsiz; ++x)
    {
      SDL_BlitSurface(tileSurfaces[TILE_WALL], nullptr, textureSurf, &rect);
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
  SDL_Surface* textureSurf = nullptr;
  SDL_LockTextureToSurface(texture, nullptr, &textureSurf);
  SDL_Rect rect = { 0, 0, TILE_SZ_PX ,TILE_SZ_PX };
  for (uint8_t y = 0; y < ysiz; ++y)
  {
    for (uint8_t x = 0; x < xsiz; ++x)
    {
      const bhMap::Block* block = map->GetBlock(x, ysiz - (y + 1));
      //SDL_BlitSurface(block->Solid() ? tileSurfaces[TILE_WALL] : tileSurfaces[TILE_FREE], nullptr, textureSurf, &rect);
      if (block->Solid())
      {
        SDL_BlitSurface(tileSurfaces[TILE_WALL], nullptr, textureSurf, &rect);
      }
      else
      {
        SDL_BlitSurface(tileSurfaces[TILE_FREE], nullptr, textureSurf, &rect);
      }
      rect.x += TILE_SZ_PX;
    }
    rect.x = 0;
    rect.y += TILE_SZ_PX;
  }
  SDL_UnlockTexture(texture);
}

void bhMapRenderer::DrawWidget(bhMap* map)
{
  ImGui::Begin("SDL_Renderer Texture Test");
  ImGui::Text("pointer = %p", texture);
  ImGui::Text("size = %d x %d", texture->w, texture->h);
  ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2((float)texture->w, (float)texture->h));

  if (ImGui::IsItemClicked())
  {
    ImVec2 mp = ImGui::GetMousePos();
    ImVec2 irm = ImGui::GetItemRectMin();
    //ImVec2 irs = ImGui::GetItemRectSize();

    uint8_t x = uint8_t(bhMath::Max(0.0f, mp.x - irm.x)) / TILE_SZ_PX;
    uint8_t y = uint8_t(bhMath::Max(0.0f, mp.y - irm.y)) / TILE_SZ_PX;

    bhMap::Block* block = map->GetBlock(x, ysiz - (y + 1));
    block->ToggleSolid();
    DrawLayout(map);

    SDL_Log("Mouse coords %d %d\n", x, y);
  }

  ImGui::End();
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
