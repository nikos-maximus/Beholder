#ifndef BH_MESH_CACHE_HPP
#define BH_MESH_CACHE_HPP

#include <unordered_map>
#include "bhHash.hpp"
#include "bhResource.hpp"
#include "Mesh/bhMesh.hpp"

class bhMeshCache
{
public:
  bhMeshCache() = default;
  void Init();
  bhMesh* CreateMesh(const char* name);
  bhMesh* GetMesh(const char* name);
  bhMesh* GetMesh(bhHash_t hash);
  bool DeleteMesh(bhHash_t hash);
  //const std::unordered_map<bhHash_t, bhMesh>& GetMeshes() const { return meshes; }

protected:
private:
  std::unordered_map<bhHash_t, bhResourceInfo> hashes;
  std::unordered_map<bhHash_t, bhMesh> meshes;
};

#endif //BH_MESH_CACHE_HPP
