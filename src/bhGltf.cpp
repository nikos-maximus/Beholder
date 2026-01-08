#include <tiny_gltf.h>
#include "bhLog.hpp"
#include "bhVec2.hpp"
#include "bhVec3.hpp"
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
  bool ImportMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model)
  {
    size_t numPatches = mesh.primitives.size();
    std::vector<std::vector<bhVec3f>> tmpPositions(numPatches);
    std::vector<std::vector<bhVec3f>> tmpNormals(numPatches);
    std::vector<std::vector<bhVec3f>> tmpTangents(numPatches);
    std::vector<std::vector<bhVec2f>> tmpUV0(numPatches);
    //std::vector<std::vector<bhVec2f>> tmpTexcoord_1(numPatches);

    std::vector<std::vector<uint32_t>> tmpInds(numPatches);

    for (size_t primitiveIndex = 0; primitiveIndex < mesh.primitives.size(); ++primitiveIndex)
    {
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
            //bindingBits |= BINDING_MASK_POSITIONS;
            tmpPositions[primitiveIndex].resize(accessor.count);
            memcpy(tmpPositions[primitiveIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
            break;
          }
          case ATTR_TYPE_NORMAL:
          {
            CheckAccessorData(accessor, TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT);
            //bindingBits |= BINDING_MASK_NORMALS;
            tmpNormals[primitiveIndex].resize(accessor.count);
            memcpy(tmpNormals[primitiveIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
            break;
          }
          case ATTR_TYPE_TANGENT:
          {
            CheckAccessorData(accessor, TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT);
            //bindingBits |= BINDING_MASK_TANGENTS;
            tmpTangents[primitiveIndex].resize(accessor.count);
            memcpy(tmpTangents[primitiveIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
            break;
          }
          case ATTR_TYPE_TEXCOORD_0:
          {
            CheckAccessorData(accessor, TINYGLTF_TYPE_VEC2, TINYGLTF_COMPONENT_TYPE_FLOAT);
            //bindingBits |= BINDING_MASK_UV_0;
            tmpUV0[primitiveIndex].resize(accessor.count);
            memcpy(tmpUV0[primitiveIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec2f));
            break;
          }
          //case ATTR_TYPE_TEXCOORD_1:
          //{
          //  CheckAccessorData(accessor, TINYGLTF_TYPE_VEC2, TINYGLTF_COMPONENT_TYPE_FLOAT);
          // //bindingBits |= BINDING_MASK_TEXCOORD_1;
          // tmpTexcoord_1[primitiveIndex].resize(accessor.count);
          // memcpy(tmpTexcoord_1[primitiveIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec2f));
          // break;
          //}
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

      //Read Indices
      tinygltf::Accessor const& accessor = model.accessors[primitive.indices];
      tinygltf::BufferView const& bufferView = model.bufferViews[accessor.bufferView];
      tinygltf::Buffer const& buffer = model.buffers[bufferView.buffer];

      CheckAccessorData(accessor, TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);
      tmpInds[primitiveIndex].resize(accessor.count);
      memcpy(tmpInds[primitiveIndex].data(), buffer.data.data(), accessor.count * sizeof(uint32_t));
    }

    bhMesh* newMesh = new bhMesh();
    newMesh->Create();

    ////Calculate number of vertices
    //for (auto const& tp : tmpPositions)
    //{
    //  numVerts += tp.size();
    //}
    ////Check for proper number of normals
    //uint16_t numNormals = 0;
    //for (auto const& tn : tmpNormals)
    //{
    //  numNormals += tn.size();
    //}
    //assert(numVerts == numNormals);
    ////Check for proper number of texcoords_0
    //uint16_t numTexcoords_0 = 0;
    //for (auto const& ttc0 : tmpTexcoord_0)
    //{
    //  numTexcoords_0 += ttc0.size();
    //}
    //assert(numVerts == numTexcoords_0);

    //AllocateMem();

    //uint16_t vertexOffset = 0;
    //for (uint8_t p = 0; p < numPatches; ++p)
    //{
    //  memcpy(positions + vertexOffset, tmpPositions[p].data(), tmpPositions[p].size() * sizeof(bhVec3f));
    //  //vertexOffset += tp.size();
    //}

    return true;
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
