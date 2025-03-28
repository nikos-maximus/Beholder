#ifndef BH_MESH_CACHE_HPP
#define BH_MESH_CACHE_HPP

#include <string>
#include "bhHash.hpp"
#include "Mesh/bhMesh.hpp"
#include "VK/bhMeshVk.hpp"

class bhMesh;

namespace bhMeshCache
{
  struct Resource
  {
    bhMesh hostMesh;
    bhVk::Mesh deviceMesh;
  };

  void Init();
  void Destroy();
  //std::string GetPath(bhHash_t hash);
  const Resource* Get(bhHash_t hash);
  const Resource* Get(const char* path);

  Resource* New(const std::string& path);
  bool CreateDeviceMesh(Resource& res);
};

#endif //BH_MESH_CACHE_HPP
