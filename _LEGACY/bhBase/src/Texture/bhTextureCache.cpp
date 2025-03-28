#include "Texture/bhTextureCache.hpp"
#include "Platform/bhPlatform.hpp"

std::unique_ptr<bhTextureCache> bhTextureCache::s_textureCache;

std::unique_ptr<bhTextureCache>& bhTextureCache::Get()
{
	if (!s_textureCache)
	{
		s_textureCache.reset(new bhTextureCache());
	}
	return s_textureCache;
}

void bhTextureCache::Destroy(std::function<void(DeviceTexture&)> deviceTextureDestryFunc)
{
	for (auto& t : s_textureCache->textures)
	{
		deviceTextureDestryFunc(t.second.GetResource()->GetTexture());
	}
	s_textureCache.reset();
}

void bhTextureCache::InitPathContents()
{
	const char* textureDir = bhPlatform::GetResourceDir(bhPlatform::RT_TEXTURE);
	char const* extensions[] = { "png", nullptr };
	bhResource_AddDirectory<bhTexture>(textures, textureDir, extensions);
}

TextureResource* bhTextureCache::GetTexture(bhHash_t hash)
{
	auto res = textures.find(hash);
	if (res == textures.end())
	{
		return nullptr;
	}
	auto& mr = res->second;
	switch (mr.GetStatus())
	{
		case TextureResource::RESOURCE_READY:
		{
			//mr.IncRef();
			return &(mr);
		}
		case TextureResource::RESOURCE_NA:
		{
			mr.SetPending();
			if (mr.GetResource()->Load(mr.GetName().c_str()))
			{
				mr.SetReady();
				return &(mr);
			}
			mr.ResetStatus();
			return nullptr;
		}
		case TextureResource::RESOURCE_PENDING:
		{
			return &(mr); // TODO: How to handle this on caller's end?
		}
		default: // Spurious status value
		{
			assert(false);
			break;
		}
	}
	return nullptr;
}

TextureResource* bhTextureCache::GetTexture(const char* name)
{
	return GetTexture(bhHash(name));
}

bool bhTextureCache::DeleteTexture(bhHash_t hash)
{
	return textures.erase(hash) > 0;
}
