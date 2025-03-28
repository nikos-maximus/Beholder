#include <SDL3/SDL_assert.h>
#include <cgltf.h>
#include "Mesh/bhMesh.hpp"

static constexpr uint16_t REQ_WORLD_VERTEX_FORMAT =
  BH_BIT(cgltf_attribute_type_normal) |
  //BH_BIT(cgltf_attribute_type_tangent) |
  //BH_BIT(cgltf_attribute_type_texcoord) |
  BH_BIT(cgltf_attribute_type_position);

__forceinline bool IsValidPosition(const cgltf_accessor* acc) //Vec3f
{
  return acc->type == cgltf_type_vec3 && acc->component_type == cgltf_component_type_r_32f;
}

__forceinline bool IsValidNormal(const cgltf_accessor* acc) //Vec3f
{
  return acc->type == cgltf_type_vec3 && acc->component_type == cgltf_component_type_r_32f;
}

__forceinline bool IsValidTangent(const cgltf_accessor* acc) //Vec3f
{
  return acc->type == cgltf_type_vec3 && acc->component_type == cgltf_component_type_r_32f;
}

__forceinline bool IsValidTexCoord(const cgltf_accessor* acc) //Vec2f
{
  return acc->type == cgltf_type_vec2 && acc->component_type == cgltf_component_type_r_32f;
}

__forceinline bool IsValidIndex(const cgltf_accessor* acc) //unsigned 16/32-bit int
{
  if (acc->type == cgltf_type_scalar)
  {
  #if BH_USE_MESH_INDEX_TYPE_UNIT16
    return (acc->component_type == cgltf_component_type_r_16u);
  #else
    return (acc->component_type == cgltf_component_type_r_32u) || (acc->component_type == cgltf_component_type_r_16u);
  #endif
  }
  return false;
}

bool IsValid(const cgltf_mesh& gltfm, uint16_t reqVertexFormat)
{
  uint16_t formatAttributes = 0;
  cgltf_size attributeSizes[cgltf_attribute_type_max_enum] = {};

  for (cgltf_size p = 0; p < gltfm.primitives_count; ++p)
  {
    const cgltf_primitive& currPrim = gltfm.primitives[p];
    size_t test = sizeof(formatAttributes);
    formatAttributes = 0;
    for (cgltf_size a = 0; a < currPrim.attributes_count; ++a)
    {
      const cgltf_attribute& currAttr = currPrim.attributes[a];
      formatAttributes |= BH_BIT(currAttr.type);
      attributeSizes[currAttr.type] = currAttr.data->count;
    }
    if (formatAttributes != reqVertexFormat)
    {
      return false;
    }

    // Check whether Normals, UVs, Tangents etc. are as many as verts
    bool attributeCountMatchesVerts = true;
    cgltf_size numVerts = attributeSizes[cgltf_attribute_type_position];
    for (auto attr = cgltf_attribute_type_position + 1; attr < cgltf_attribute_type_max_enum; ++attr)
    {
      if (formatAttributes & BH_BIT(attr))
      {
        attributeCountMatchesVerts &= numVerts == attributeSizes[attr];
      }
    }

    if (!attributeCountMatchesVerts)
    {
      return false;
    }
  }
  return true;
}

bool bhMesh::ImportMesh(bhMesh& mesh_bh, const cgltf_mesh& gltfMesh)
{
  //Test required mesh data
  if (!IsValid(gltfMesh, REQ_WORLD_VERTEX_FORMAT)) return false;

  size_t totalVerts = 0, totalInds = 0;
  for (cgltf_size p = 0; p < gltfMesh.primitives_count; ++p)
  {
    const cgltf_primitive& prim = gltfMesh.primitives[p];
    SDL_assert(prim.attributes_count > 0);
    totalVerts += prim.attributes[0].data->count; //Here we assume that all vertex-related attributes (positions, normals) etc. have same number of entries
    totalInds += prim.indices->count;
  }

  SDL_assert((totalVerts <= UINT32_MAX && totalInds <= UINT32_MAX) && gltfMesh.primitives_count <= UINT32_MAX);

  mesh_bh._Allocate(static_cast<uint32_t>(totalVerts), static_cast<uint32_t>(totalInds), static_cast<uint32_t>(gltfMesh.primitives_count));
  uint32_t baseVert = 0, baseInd = 0;
  uint32_t primVerts = 0, primInds = 0;
  for (cgltf_size p = 0; p < gltfMesh.primitives_count; ++p)
  {
    const cgltf_primitive& gltfPrim = gltfMesh.primitives[p];
    bhMesh::Primitive& primitive_bh = mesh_bh.GetPrimitive(static_cast<uint32_t>(p));

    primitive_bh.baseVertex = baseVert;
    SDL_assert(gltfPrim.attributes[0].data->count <= UINT32_MAX);
    primitive_bh.numVerts = static_cast<uint32_t>(gltfPrim.attributes[0].data->count);
    baseVert += primitive_bh.numVerts;

    for (cgltf_size a = 0; a < gltfPrim.attributes_count; ++a)
    {
      const cgltf_attribute& currAttr = gltfPrim.attributes[a];
      uint8_t* bufferData = static_cast<uint8_t*>(currAttr.data->buffer_view->buffer->data);
      switch (currAttr.type)
      {
        case cgltf_attribute_type_position:
        {
          if (!IsValidPosition(currAttr.data))
          {
            SDL_assert(false);
          }
          glm::vec3* vec3Positions = reinterpret_cast<glm::vec3*>(bufferData + currAttr.data->offset);
          for (size_t v = 0; v < currAttr.data->count; ++v)
          {
            mesh_bh.verts[primitive_bh.baseVertex + v].position = vec3Positions[v];
          }
          mesh_bh.config.AddBinding(bhMeshBindings::MESH_BINDING_POSITIONS);
          break;
        }
        case cgltf_attribute_type_normal:
        {
          if (!IsValidNormal(currAttr.data))
          {
            SDL_assert(false);
          }
          glm::vec3* vec3Normals = reinterpret_cast<glm::vec3*>(bufferData + currAttr.data->offset);
          for (size_t v = 0; v < currAttr.data->count; ++v)
          {
            mesh_bh.verts[primitive_bh.baseVertex + v].normal = vec3Normals[v];
          }
          mesh_bh.config.AddBinding(bhMeshBindings::MESH_BINDING_NORMALS);
          break;
        }
        case cgltf_attribute_type_tangent:
        {
          if (!IsValidTangent(currAttr.data))
          {
            SDL_assert(false);
          }
          glm::vec3* vec3Tangents = reinterpret_cast<glm::vec3*>(bufferData + currAttr.data->offset);
          for (size_t v = 0; v < currAttr.data->count; ++v)
          {
            mesh_bh.verts[primitive_bh.baseVertex + v].tangent = vec3Tangents[v];
          }
          mesh_bh.config.AddBinding(bhMeshBindings::MESH_BINDING_TANGENTS);
          break;
        }
        case cgltf_attribute_type_texcoord:
        {
          if (!IsValidTexCoord(currAttr.data))
          {
            SDL_assert(false);
          }
          glm::vec2* vec2TexCoords = reinterpret_cast<glm::vec2*>(bufferData + currAttr.data->offset);
          for (size_t v = 0; v < currAttr.data->count; ++v)
          {
            mesh_bh.verts[primitive_bh.baseVertex + v].uv_0 = vec2TexCoords[v];
          }
          mesh_bh.config.AddBinding(bhMeshBindings::MESH_BINDING_UV_0);
          break;
        }
        default:
        {
          break;
        }
      }
    }

    primitive_bh.baseIndex = baseInd;
    SDL_assert(gltfPrim.indices->count <= UINT32_MAX);
    primitive_bh.numInds = static_cast<uint32_t>(gltfPrim.indices->count);
    baseInd += primitive_bh.numInds;
    if (!IsValidIndex(gltfPrim.indices))
    {
      SDL_assert(false);
    }

    uint8_t* bufferData = static_cast<uint8_t*>(gltfPrim.indices->buffer_view->buffer->data);
    bufferData += gltfPrim.indices->buffer_view->offset;
    switch (gltfPrim.indices->component_type)
    {
      case cgltf_component_type_r_16u:
      {
        uint16_t* inds16 = reinterpret_cast<uint16_t*>(bufferData);
        for (cgltf_size i = 0; i < gltfPrim.indices->count; ++i)
        {
          mesh_bh.inds[primitive_bh.baseIndex + i] = inds16[i];
        }
        break;
      }
      case cgltf_component_type_r_32u:
      {
        uint32_t* inds32 = reinterpret_cast<uint32_t*>(bufferData);
        for (cgltf_size i = 0; i < gltfPrim.indices->count; ++i)
        {
          mesh_bh.inds[primitive_bh.baseIndex + i] = inds32[i];
        }
        break;
      }
      case cgltf_component_type_r_8:
      case cgltf_component_type_r_8u:
      case cgltf_component_type_r_16:
      case cgltf_component_type_r_32f:
      {
        SDL_assert(false);
      }
    }
  }
  
  return true;
}
