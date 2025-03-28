#ifndef BH_MESH_CACHE_HPP
#define BH_MESH_CACHE_HPP

#include <unordered_map>
#include "bhHash.hpp"
#include "bhResource.hpp"
#include "Mesh/bhMesh.hpp"

typedef bhResource<bhMesh> MeshResource;

class bhMeshCache
{
public:
	void InitPathContents();
	MeshResource* CreateMesh(const char* name);
	MeshResource* GetMesh(const char* name);
	MeshResource* GetMesh(bhHash_t hash);
	bool DeleteMesh(bhHash_t hash);
	const std::unordered_map<bhHash_t, MeshResource>& GetMeshes() const { return meshes; }

	static std::unique_ptr<bhMeshCache>& Get();
	static void Destroy();

protected:
private:
	bhMeshCache() = default;
	std::unordered_map<bhHash_t, MeshResource> meshes;

	static std::unique_ptr<bhMeshCache> s_meshCache;
};

#endif //BH_MESH_CACHE_HPP
