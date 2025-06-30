#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_video.h>
#include "bhPlatform.hpp"
#include "bhImage.hpp"
#include "bhSDL3.hpp"

namespace bhSDL3
{
  SDL_Surface* CreateSurfaceFromImageFile(const char* fileName)
  {
    static constexpr int NUM_IMG_COMPONENTS = 4;
#ifdef SDL_PLATFORM_WINDOWS
    static constexpr SDL_PixelFormat IMG_PIXEL_FORMAT = SDL_PIXELFORMAT_ABGR8888;
#else
    static constexpr SDL_PixelFormat IMG_PIXEL_FORMAT = SDL_PIXELFORMAT_ARGB8888; // TODO: Needs testing
#endif

    const char* filePath = bhPlatform::CreateResourcePath(bhPlatform::ResourceType::RT_IMAGE, fileName);
    bhImage* img_tile = bhImage::CreateFromFile(filePath, NUM_IMG_COMPONENTS);
    delete[] filePath;

    if (img_tile)
    {
      SDL_Surface* newSurf = SDL_CreateSurface(img_tile->Width(), img_tile->Height(), IMG_PIXEL_FORMAT);
      memcpy(newSurf->pixels, img_tile->Pixels(), img_tile->MemSiz());
      bhImage::Destroy(img_tile);
      return newSurf;
    }

    SDL_assert(false);
    return nullptr;
  }
}
