#include <SDL3/SDL_video.h>
#include "bhPlatform.hpp"
#include "bhImage.hpp"
#include "bhSDL3.hpp"

namespace bhSDL3
{
  SDL_Surface* CreateSurfaceFromImageFile(const char* fileName)
  {
    const char* filePath = bhPlatform::CreateResourcePath(bhPlatform::ResourceType::RT_IMAGE, fileName);
    bhImage* img_tile = bhImage::CreateFromFile(filePath, 4);
    delete[] filePath;
    
    SDL_Surface* newSurf = SDL_CreateSurface(img_tile->Width(), img_tile->Height(), SDL_PIXELFORMAT_ARGB8888);
    memcpy(newSurf->pixels, img_tile->Pixels(), img_tile->MemSiz());
    bhImage::Destroy(img_tile);
    
    return newSurf;
  }
}
