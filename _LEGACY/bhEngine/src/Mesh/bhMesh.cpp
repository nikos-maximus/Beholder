#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>
#include "Mesh/bhMesh.hpp"
#include "bhTypes.hpp"
#include "Platform/bhPlatform.hpp"
#include "bhLog.h"

////////////////////////////////////////////////////////////////////////////////
void bhMesh::_Allocate(uint32_t _numVerts, uint32_t _numInds, uint32_t _numPrims)
{
  numVerts = _numVerts;
  verts = (bhMeshVertex_t*)calloc(numVerts, sizeof(bhMeshVertex_t));

  numInds = _numInds;
  inds = (bhMeshIdx_t*)calloc(numInds, sizeof(bhMeshIdx_t));

  numPrims = _numPrims;
  prims = (Primitive*)calloc(numPrims, sizeof(Primitive));

  //deviceData = {};
}

void bhMesh::_Free()
{
  free(prims);
  prims = nullptr;
  numPrims = 0;

  free(inds);
  inds = nullptr;
  numInds = 0;

  free(verts);
  verts = nullptr;
  numVerts = 0;
}

bhMesh::bhMesh(uint32_t _numVerts, uint32_t _numInds, uint32_t _numPrims)
{
  _Allocate(_numVerts, _numInds, _numPrims);
}

//bhMesh::bhMesh(bhMesh&& _mesh)
//  : setup(_mesh.setup)
//  , verts(_mesh.verts)
//  , inds(_mesh.inds)
//  , prims(_mesh.prims)
//  , numVerts(_mesh.numVerts)
//  , numInds(_mesh.numInds)
//  , numPrims(_mesh.numPrims)
//{
//  _mesh._Free();
//}

bhMesh::~bhMesh()
{
  _Free();
}

bool bhMesh::Load(const char* fileName)
{
  const char* path = bhPlatform::CreateResourcePath(bhPlatform::RT_MESH, fileName);
  FILE* file = nullptr;
  errno_t err = fopen_s(&file, path, "rb");
  if (err) // zero = success
  {
    bhLog_Message(bhLogPriority::LP_ERROR, "Error opening mesh file for reading: %s", path);
    bhPlatform::FreePath(path);
    return false;
  }
  bhPlatform::FreePath(path);
  assert(file); // If the above check passes, this should never fail

  int mask = 0;
  fread(&mask, sizeof(mask), 1, file);
  fread(&numVerts, sizeof(numVerts), 1, file);
  fread(&numInds, sizeof(numInds), 1, file);
  fread(&numPrims, sizeof(numPrims), 1, file);

  config.SetMask(mask);
  _Allocate(numVerts, numInds, numPrims);
  fread(verts, sizeof(bhMeshVertex_t), numVerts, file);
  fread(inds, sizeof(bhMeshIdx_t), numInds, file);
  fread(inds, sizeof(Primitive), numPrims, file);

  fclose(file);
  return true;
}

bool bhMesh::Save(const char* fileName) const
{
  const char* path = bhPlatform::CreateResourcePath(bhPlatform::RT_MESH, fileName, bhPlatform::GetFileExtension(bhPlatform::RT_MESH));
  FILE* file = nullptr;
  errno_t err = fopen_s(&file, path, "wb");
  if (err) // zero = success
  {
    bhLog_Message(bhLogPriority::LP_ERROR, "Error opening mesh file for writing: %s", path);
    bhPlatform::FreePath(path);
    return false;
  }
  bhPlatform::FreePath(path);
  assert(file); // If the above check passes, this should never fail

  int mask = config.GetMask();
  fwrite(&mask, sizeof(mask), 1, file);
  fwrite(&numVerts, sizeof(numVerts), 1, file);
  fwrite(&numInds, sizeof(numInds), 1, file);
  fwrite(&numPrims, sizeof(numPrims), 1, file);

  fwrite(verts, sizeof(bhMeshVertex_t), numVerts, file);
  fwrite(inds, sizeof(bhMeshIdx_t), numInds, file);
  fwrite(prims, sizeof(Primitive), numPrims, file);

  fclose(file);
  return true;
}

int bhMesh_GetNumBindings(int bindingsMask)
{
  int numBindings = 0;
  for (int i = 0; i < NUM_MESH_BINDINGS; ++i)
  {
    if (bindingsMask & BH_BIT(i))
    {
      ++numBindings;
    }
  }
  return numBindings;
}

void bhMesh::ApplyVertexOffset(float xOffs, float yOffs, float zOffs)
{
  for (uint32_t v = 0; v < numVerts; ++v)
  {
    verts[v].position.x = xOffs;
    verts[v].position.y = yOffs;
    verts[v].position.z = zOffs;
  }
}

void bhMesh::ApplyVertexOffset(glm::vec3 offs)
{
  ApplyVertexOffset(offs.x, offs.y, offs.z);
}

void bhMesh::ScaleUV(float xScale, float yScale)
{
  for (uint32_t v = 0; v < numVerts; ++v)
  {
    verts[v].uv_0.x *= xScale;
    verts[v].uv_0.y *= yScale;
  }
}

void bhMesh::ScaleUV(glm::vec2 scale)
{
  ScaleUV(scale.x, scale.y);
}

void bhMesh::ScaleUV(float uScale)
{
  ScaleUV(uScale, uScale);
}

bhMesh::Primitive& bhMesh::GetPrimitive(uint32_t idx)
{
  SDL_assert(idx < numPrims);
  return prims[idx];
}

const bhMesh::Primitive& bhMesh::GetPrimitive(uint32_t idx) const
{
  SDL_assert(idx < numPrims);
  return prims[idx];
}
