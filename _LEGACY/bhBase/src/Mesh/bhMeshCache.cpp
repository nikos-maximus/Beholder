#include "Mesh/bhMeshCache.hpp"
#include "Platform/bhPlatform.hpp"

std::unique_ptr<bhMeshCache> bhMeshCache::s_meshCache;

std::unique_ptr<bhMeshCache>& bhMeshCache::Get()
{
	if (!s_meshCache)
	{
		s_meshCache.reset(new bhMeshCache());
	}
	return s_meshCache;
}

void bhMeshCache::Destroy()
{
	s_meshCache.reset();
}

void bhMeshCache::InitPathContents()
{
	const char* meshDir = bhPlatform::GetResourceDir(bhPlatform::RT_MESH);
	//char const* extensions[] = { "glb", "gltf", nullptr };
	char const* extensions[] = { "bms", nullptr };
	bhResource_AddDirectory<bhMesh>(meshes, meshDir, extensions);
}

MeshResource* bhMeshCache::CreateMesh(const char* name)
{
	bhHash_t hash = bhHash(name);
	auto res = meshes.find(hash);
	if (res != meshes.end())
	{
		return &(res->second);
	}

	auto result = meshes.emplace(hash, MeshResource(name));
	return &(result.first->second); //result.first is meshes::iterator
}

MeshResource* bhMeshCache::GetMesh(bhHash_t hash)
{
	auto res = meshes.find(hash);
	if (res == meshes.end())
	{
		return nullptr;
	}
	auto& mr = res->second;
	switch (mr.GetStatus())
	{
		case MeshResource::RESOURCE_READY:
		{
			//mr.IncRef();
			return &(mr);
		}
		case MeshResource::RESOURCE_NA:
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
		case MeshResource::RESOURCE_PENDING:
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

MeshResource* bhMeshCache::GetMesh(const char* name)
{
	return GetMesh(bhHash(name));
}

bool bhMeshCache::DeleteMesh(bhHash_t hash)
{
	return meshes.erase(hash) > 0;
}
