#include "Mesh/bhMeshCache.hpp"
#include "VK/bhDeviceVk.hpp"
#include "VK/bhMeshVk.hpp"
#include "Mesh/bhMesh.hpp"
#include "bhResource.hpp"

namespace bhMeshCache
{
  bhResourceCache<Resource> g_cache;

  void Init()
  {}

  void MeshDestroyFunc(std::unique_ptr<Resource>& mb)
  {
    bhVk::DestroyDeviceMesh(bhVk::GetRenderDevice(), mb->deviceMesh);
  }

  void Destroy()
  {
    g_cache.ClearAll(MeshDestroyFunc);
  }

  //std::string GetPath(bhHash_t hash) {}

  const Resource* Get(bhHash_t hash)
  {
    return g_cache.Get(hash);
  }

  const Resource* Get(const char* path)
  {
    return g_cache.Get(path);
  }

  Resource* New(const std::string& path)
  {
    return g_cache.New(path);
  }

  bool CreateDeviceMesh(Resource& res)
  {
    return bhVk::CreateDeviceMesh(res.deviceMesh, bhVk::GetRenderDevice(), res.hostMesh) == VK_TRUE;
  }
}
