#include <tiny_gltf.h>
#include "bhLog.hpp"
#include "bhMesh.hpp"
#include "bhGltf.hpp"

namespace bhGltf
{
  ////////////////////////////////////////////////////////////////////////////////
  enum AttributeType
  {
    ATTR_TYPE_POSITION,
    ATTR_TYPE_NORMAL,
    ATTR_TYPE_TANGENT,
    ATTR_TYPE_TEXCOORD_0,
    ATTR_TYPE_TEXCOORD_1,
    ATTR_TYPE_COLOR_0,
    ATTR_TYPE_JOINTS_0,
    ATTR_TYPE_WEIGHTS_0,

    NUM_ATTR_TYPES,
    ATTR_TYPE_INVALID = NUM_ATTR_TYPES
  };

  AttributeType GetAttributeType(std::string const& tag)
  {
    static std::string attributeNames[NUM_ATTR_TYPES] =
    {
        "POSITION",
        "NORMAL",
        "TANGENT",
        "TEXCOORD_0",
        "TEXCOORD_1",
        "COLOR_0",
        "JOINTS_0",
        "WEIGHTS_0",
    };
    for (uint8_t i = 0; i < NUM_ATTR_TYPES; ++i)
    {
      if (attributeNames[i] == tag)
      {
        return (AttributeType)i;
      }
    }
    return ATTR_TYPE_INVALID;
  }

  inline void CheckAccessorData(const tinygltf::Accessor& accessor, int type, int componentType)
  {
    assert((accessor.type == type) && (accessor.componentType == componentType));
  }

  ////////////////////////////////////////////////////////////////////////////////
  bhMesh* ImportMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model)
  {
    size_t numPatches = mesh.primitives.size();
    std::vector<std::vector<bhMesh::Vertex>> vertsPerPatch;
    std::vector<std::vector<bhMesh::Index_t>> indsPerPatch;

    for (size_t primitiveIndex = 0; primitiveIndex < mesh.primitives.size(); ++primitiveIndex)
    {
      std::vector<bhVec3f> primPositions;
      std::vector<bhVec3f> primNormals;
      std::vector<bhVec3f> primTangents;
      std::vector<bhVec2f> primUV0;
      std::vector<bhMesh::Index_t> primInds;

      tinygltf::Primitive const& primitive = mesh.primitives[primitiveIndex];
      for (auto const& attribute : primitive.attributes)
      {
        tinygltf::Accessor const& accessor = model.accessors[attribute.second];
        tinygltf::BufferView const& bufferView = model.bufferViews[accessor.bufferView];
        tinygltf::Buffer const& buffer = model.buffers[bufferView.buffer];

        //Read Attributes
        switch (GetAttributeType(attribute.first))
        {
          case ATTR_TYPE_POSITION:
          {
            CheckAccessorData(accessor, TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT);
            primPositions.resize(accessor.count);
            memcpy(primPositions.data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
            break;
          }
          case ATTR_TYPE_NORMAL:
          {
            CheckAccessorData(accessor, TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT);
            primNormals.resize(accessor.count);
            memcpy(primNormals.data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
            break;
          }
          case ATTR_TYPE_TANGENT:
          {
            CheckAccessorData(accessor, TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT);
            primTangents.resize(accessor.count);
            memcpy(primTangents.data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
            break;
          }
          case ATTR_TYPE_TEXCOORD_0:
          {
            CheckAccessorData(accessor, TINYGLTF_TYPE_VEC2, TINYGLTF_COMPONENT_TYPE_FLOAT);
            primUV0.resize(accessor.count);
            memcpy(primUV0.data(), buffer.data.data(), accessor.count * sizeof(bhVec2f));
            break;
          }
          case ATTR_TYPE_COLOR_0:
          {
            break;
          }
          case ATTR_TYPE_JOINTS_0:
          {
            break;
          }
          case ATTR_TYPE_WEIGHTS_0:
          {
            break;
          }
          case ATTR_TYPE_INVALID:
          default:
          {
            break;
          }
        }
      }

      assert(primPositions.size() == primNormals.size() == primTangents.size() == primUV0.size());
      size_t numVerts = primPositions.size();
      std::vector<bhMesh::Vertex> primVerts(numVerts);
      for (size_t v = 0; v < numVerts; ++v)
      {
        primVerts[v].position = primPositions[v];
        primVerts[v].normal = primNormals[v];
        primVerts[v].tangent = primTangents[v];
        primVerts[v].uv0 = primUV0[v];
      }
      vertsPerPatch.push_back(std::move(primVerts));

      //Read Indices
      tinygltf::Accessor const& accessor = model.accessors[primitive.indices];
      tinygltf::BufferView const& bufferView = model.bufferViews[accessor.bufferView];
      tinygltf::Buffer const& buffer = model.buffers[bufferView.buffer];

      CheckAccessorData(accessor, TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);
      primInds.resize(accessor.count);
      memcpy(primInds.data(), buffer.data.data(), accessor.count * sizeof(bhMesh::Index_t));
      indsPerPatch.push_back(std::move(primInds));
    }

    bhMesh* newMesh = new bhMesh();
    newMesh->Create(vertsPerPatch, indsPerPatch);

    return newMesh;
  }

  bool ImportFile(const char* filePath)
  {
    tinygltf::TinyGLTF gltfContext;
    tinygltf::Model model;
    std::string err, warn;
    bool result = false;

    const std::string filePathStr(filePath);
    if (filePathStr.rfind(".gltf"))  //ASCII file
    {
      result = gltfContext.LoadASCIIFromFile(&model, &err, &warn, filePath);
    }
    else if (filePathStr.rfind(".glb"))  //Binary file
    {
      result = gltfContext.LoadBinaryFromFile(&model, &err, &warn, filePath);
    }
    else
    {
      bhLog::Message(bhLog::LOG_CATEGORY_APPLICATION, bhLog::LOG_PRIORITY_ERROR, "Could not determine format for file ", filePath);
      return false;
    }

    if (!err.empty())
    {
      bhLog::Message(bhLog::LOG_CATEGORY_APPLICATION, bhLog::LOG_PRIORITY_ERROR, err.c_str());
    }
    if (!warn.empty())
    {
      bhLog::Message(bhLog::LOG_CATEGORY_APPLICATION, bhLog::LOG_PRIORITY_WARN, warn.c_str());
    }
    if (!result)
    {
      bhLog::Message(bhLog::LOG_CATEGORY_APPLICATION, bhLog::LOG_PRIORITY_ERROR, "Failed to import file ", filePath);
      return false;
    }

    for (const auto& mesh : model.meshes)
    {
      bhMesh* newMesh = ImportMesh(mesh, model);
    }

    return true;
  }
}

//bhResourceIndex_t bhMesh::Load_gltf(char const* fileName)
//{
//  bhResourceIndex_t meshIndex = meshes.Find(fileName);
//  if ((meshIndex != BH_INVALID_INDEX) && (meshes.GetResourceStatus(meshIndex) == bhResourceStatus::RS_READY))
//  {
//    return meshIndex;
//  }
//  meshIndex = meshes.GetNextFreeResourceIndex(fileName);
//  if (meshIndex == BH_INVALID_INDEX) return BH_INVALID_INDEX;
//  bhMesh* mesh = meshes.GetResource(meshIndex);
//
//  std::string path(bhUtil::CreatePath(bhEnv::GetEnvString(bhEnv::MESHES_PATH), fileName));
//
//  if (!tinygltf::FileExists(path, nullptr))
//  {
//    return BH_INVALID_INDEX;
//  }
//  tinygltf::TinyGLTF context;
//
//  tinygltf::Model model;
//  std::string error, warning;
//  if (!context.LoadASCIIFromFile(&model, &error, &warning, path))
//  {
//    //Log errors/warnings
//    return BH_INVALID_INDEX;
//  }
//  if (!mesh->Load_gltf(model))
//  {
//    return BH_INVALID_INDEX;
//  }
//  return meshIndex;
//}
