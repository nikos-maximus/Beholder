#ifndef BH_TEXTURE_CACHE_HPP
#define BH_TEXTURE_CACHE_HPP

#include <unordered_map>
#include <functional>
#include "bhResource.hpp"
#include "bhTexture.hpp"

typedef bhResource<bhTexture> TextureResource;

class bhTextureCache
{
public:
	void InitPathContents();
	const std::unordered_map<bhHash_t, TextureResource>& GetTextures() const { return textures; }
	//TextureResource* CreateTexture(const char* name);
	TextureResource* GetTexture(const char* name);
	TextureResource* GetTexture(bhHash_t hash);
	bool DeleteTexture(bhHash_t hash);

	static std::unique_ptr<bhTextureCache>& Get();
	static void Destroy(std::function<void(DeviceTexture&)> deviceTextureDestryFunc);

protected:
private:
	bhTextureCache() = default;
	std::unordered_map<bhHash_t, TextureResource> textures;

	static std::unique_ptr<bhTextureCache> s_textureCache;
};

#endif //BH_TEXTURE_CACHE_HPP
