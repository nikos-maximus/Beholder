#include "Mesh/bhMeshCache.hpp"
#include "Platform/bhPlatform.hpp"

void bhMeshCache::Init()
{
  const char* meshDir = bhPlatform::GetResourceDir(bhPlatform::RT_MESH);
  //char const* extensions[] = { "glb", "gltf", nullptr };
  char const* extensions[] = { "bms", nullptr };
  GenerateResourceHashes(hashes, meshDir, extensions);
}

bhMesh* bhMeshCache::CreateMesh(const char* name)
{
  bhHash_t hash = bhHash(name);
  auto res = meshes.find(hash);
  if (res != meshes.end())
  {
    return &(res->second);
  }

  auto result = meshes.emplace(hash, bhMesh());
  return &(result.first->second); //result.first is meshes::iterator
}

bhMesh* bhMeshCache::GetMesh(bhHash_t hash)
{
  auto res = meshes.find(hash);
  if (res != meshes.end())
  {
    return &(res->second);
  }
  return nullptr;
}

bhMesh* bhMeshCache::GetMesh(const char* name)
{
  bhMesh* msh = GetMesh(bhHash(name));
  return msh ? msh : CreateMesh(name);
}

bool bhMeshCache::DeleteMesh(bhHash_t hash)
{
  return meshes.erase(hash) > 0;
}
