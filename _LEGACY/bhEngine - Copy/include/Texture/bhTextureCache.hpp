#ifndef BH_TEXTURE_CACHE_HPP
#define BH_TEXTURE_CACHE_HPP

#include <unordered_map>
#include "bhResource.hpp"
#include "texture/bhTexture.hpp"

class bhGPUContext;

template<class bhTexture_t>
class bhTextureCache
{
public:
	bhTextureCache() = default;

	std::unordered_map<bhHash_t, bhTexture_t>& GetTextures() { return textures; }
	//TextureResource* CreateTexture(const char* name);

	const std::string& GetPath(bhHash_t hash)
	{
		auto it = hashes.find(hash);
		assert(it != hashes.end());
		return it->second.path;
	}
	
	const bhTexture_t* GetTexture(bhHash_t hash) const
	{
		auto res = textures.find(hash);
		if (res != textures.end())
		{
			return &(res->second);
		}
		return nullptr;
	}

	inline const bhTexture_t* GetTexture(const char* path) const
	{
		return GetTexture(bhHash(path));
	}

	inline bool RemoveTexture(bhHash_t hash)
	{
		return textures.erase(hash) > 0;
	}

	void Init()
	{
		const char* textureDir = bhPlatform::GetResourceDir(bhPlatform::RT_TEXTURE);
		char const* extensions[] = { "png", nullptr };
		GenerateResourceHashes(hashes, textureDir, extensions);
	}

	bool AddTexture(bhHash_t hash, const bhTexture_t& tex)
	{
		auto it = textures.find(hash);
		if (it != textures.end())
		{
			return false;
		}
		textures.emplace(hash, tex);
		return true;
	}

protected:
private:
	std::unordered_map<bhHash_t, bhResourceInfo> hashes;
	std::unordered_map<bhHash_t, bhTexture_t> textures;
};

#endif //BH_TEXTURE_CACHE_HPP
