#include <cgltf.h>
#include "Game/bhWorld.hpp"
#include "Mesh/bhMeshCache.hpp"
#include "bhLog.h"

void bhWorld::LogAsset(const cgltf_asset& ast)
{
  bhLog_Message(bhLogPriority::LP_INFO, "Copyright: %s", ast.copyright ? ast.copyright : "N/A");
  bhLog_Message(bhLogPriority::LP_INFO, "Generator: %s", ast.generator ? ast.generator : "N/A");
  bhLog_Message(bhLogPriority::LP_INFO, "Version: %s", ast.version ? ast.version : "N/A");
  bhLog_Message(bhLogPriority::LP_INFO, "Min. Version: %s", ast.min_version ? ast.min_version : "N/A");
  //bhLog_Message(bhLogPriority::LP_INFO, "extras.data: %s", ast.extras.data);
  bhLog_Message(bhLogPriority::LP_INFO, "Extensions (%d):", ast.extensions_count);
  for (cgltf_size e = 0; e < ast.extensions_count; ++e)
  {
    bhLog_Message(bhLogPriority::LP_INFO, "\t%s", ast.extensions[e]);
  }
}

bool bhWorld::Import(const char* filePath)
{
  cgltf_options options = {};
  cgltf_data* data = nullptr;
  
  cgltf_result result = cgltf_parse_file(&options, filePath, &data);
  if (result != cgltf_result_success)
  {
    bhLog_Message(bhLogPriority::LP_ERROR, "Error parsing gltf file %s", filePath);
    // TODO: Log specific error by type
    return false;
  }

  result = cgltf_validate(data);
  if (result != cgltf_result_success)
  {
    bhLog_Message(bhLogPriority::LP_ERROR, "Error validating gltf file %s", filePath);
    // TODO: Log specific error by type
    return false;
  }

  result = cgltf_load_buffers(&options, data, filePath);
  if (result != cgltf_result_success)
  {
    bhLog_Message(bhLogPriority::LP_ERROR, "Error loading external buffers for gltf file %s", filePath);
    // TODO: Log specific error by type
    return false;
  }

  LogAsset(data->asset);

  //for (cgltf_size s = 0; s < data->scenes_count; ++s)
  //{
  //  data->scene[s];
  //}

  for (cgltf_size m = 0; m < data->meshes_count; ++m)
  {
    const cgltf_mesh& gltfMesh = data->meshes[m];
    bhMeshCache::Resource* newMesh = bhMeshCache::New(gltfMesh.name);
    if (newMesh)
    {
      if (bhMesh::ImportMesh(newMesh->hostMesh, gltfMesh))
      {
        bhMeshCache::CreateDeviceMesh(*newMesh);
      }
      //newMesh->hostMesh.Save(gltfMesh.name);
    }
  }

  cgltf_free(data);
  return true;
}
