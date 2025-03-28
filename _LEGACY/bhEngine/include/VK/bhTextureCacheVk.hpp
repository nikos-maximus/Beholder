#ifndef BH_TEXTURE_CACHE_HPP
#define BH_TEXTURE_CACHE_HPP

#include "VK/bhTextureVk.hpp"

namespace bhVk
{
  namespace TextureCache
  {
    void Init();
    void Destroy();
    //std::string GetPath(bhHash_t hash);
    const Texture* Get(bhHash_t hash);
    const Texture* Get(const char* path);

    Texture* New(const std::string& path);
  }
}

#endif //BH_TEXTURE_CACHE_HPP
