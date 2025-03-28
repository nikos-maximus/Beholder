#include "VK/bhTextureCacheVk.hpp"
#include "VK/bhDeviceVk.hpp"
#include "bhResource.hpp"

namespace bhVk
{
  namespace TextureCache
  {
    bhResourceCache<bhVk::Texture> g_cache;

    void Init()
    {}

    void TextureDestroyFunc(std::unique_ptr<Texture>& tex)
    {
      bhVk::DestroyTexture(bhVk::GetRenderDevice(), *tex);
    }

    void Destroy()
    {
      g_cache.ClearAll(TextureDestroyFunc);
    }

    //std::string GetPath(bhHash_t hash) {}

    const Texture* Get(bhHash_t hash)
    {
      return g_cache.Get(hash);
    }

    const Texture* Get(const char* path)
    {
      return g_cache.Get(path);
    }

    Texture* New(const std::string& path)
    {
      return g_cache.New(path);
    }
  }
}
