#include "bhGltf.hpp"
#include <fstream>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE 
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

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

    ////////////////////////////////////////////////////////////////////////////////
    bhMesh_gltf::bhMesh_gltf()
    {}

    bhMesh_gltf::~bhMesh_gltf()
    {
        FreeMem();
    }

    bool bhMesh_gltf::Import(tinygltf::Model const& model, size_t meshIndex)
    {
        tinygltf::Mesh const& mesh = model.meshes[meshIndex];

        numPatches = (uint8_t)mesh.primitives.size();
        std::vector<std::vector<bhVec3f>> tmpPositions(numPatches);
        std::vector<std::vector<bhVec3f>> tmpNormals(numPatches);
        //std::vector<std::vector<glm::vec4>> tmpTangents(numPatches);
        std::vector<std::vector<bhVec2f>> tmpTexcoord_0(numPatches);
        //std::vector<std::vector<bhVec2f>> tmpTexcoord_1(numPatches);

        for (size_t primitiveIndex = 0; primitiveIndex < mesh.primitives.size(); ++primitiveIndex)
        {
            tinygltf::Primitive const& primitive = mesh.primitives[primitiveIndex];
            for (auto const& attribute : primitive.attributes)
            {
                tinygltf::Accessor const& accessor = model.accessors[attribute.second];
                tinygltf::BufferView const& bufferView = model.bufferViews[accessor.bufferView];
                tinygltf::Buffer const& buffer = model.buffers[bufferView.buffer];
                // Read Attributes
                switch (GetAttributeType(attribute.first))
                {
                    case ATTR_TYPE_POSITION:
                    {
                        assert(accessor.type == TINYGLTF_TYPE_VEC3);
                        bindingBits |= BINDING_MASK_POSITIONS;
                        tmpPositions[meshIndex].resize(accessor.count);
                        memcpy(tmpPositions[meshIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
                        break;
                    }
                    case ATTR_TYPE_NORMAL:
                    {
                        assert(accessor.type == TINYGLTF_TYPE_VEC3);
                        bindingBits |= BINDING_MASK_NORMALS;
                        tmpNormals[meshIndex].resize(accessor.count);
                        memcpy(tmpNormals[meshIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec3f));
                        break;
                    }
                    case ATTR_TYPE_TANGENT:
                    {
                        bindingBits |= BINDING_MASK_TANGENTS;
                        break;
                    }
                    case ATTR_TYPE_TEXCOORD_0:
                    {
                        bindingBits |= BINDING_MASK_UV_0;
                        assert(accessor.type == TINYGLTF_TYPE_VEC2);
                        tmpTexcoord_0[meshIndex].resize(accessor.count);
                        memcpy(tmpTexcoord_0[meshIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec2f));
                        break;
                    }
                    case ATTR_TYPE_TEXCOORD_1:
                    {
                        //bindingBits |= BINDING_MASK_TEXCOORD_1;
                        //assert(accessor.type == TINYGLTF_TYPE_VEC2);
                        //tmpTexcoord_1[meshIndex].resize(accessor.count);
                        //memcpy(tmpTexcoord_1[meshIndex].data(), buffer.data.data(), accessor.count * sizeof(bhVec2f));
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
            // Read Indices
            //auto const& indices = primitive.indices;
            //if (indices != primitive.end())
            //{
            //    assert(indices.value().is_number_integer());
            //    int accessorIndex = indices.value();
            //    auto const& accessor = accessors[accessorIndex];
            //}
        }

        // Calculate number of vertices
        for (auto const& tp : tmpPositions)
        {
            numVerts += tp.size();
        }
        // Check for proper number of normals
        uint16_t numNormals = 0;
        for (auto const& tn : tmpNormals)
        {
            numNormals += tn.size();
        }
        assert(numVerts == numNormals);
        // Check for proper number of texcoords_0
        uint16_t numTexcoords_0 = 0;
        for (auto const& ttc0 : tmpTexcoord_0)
        {
            numTexcoords_0 += ttc0.size();
        }
        assert(numVerts == numTexcoords_0);

        AllocateMem();

        uint16_t vertexOffset = 0;
        for (uint8_t p = 0; p < numPatches; ++p)
        {
            memcpy(positions + vertexOffset, tmpPositions[p].data(), tmpPositions[p].size() * sizeof(bhVec3f));
            //vertexOffset += tp.size();
        }

        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool Import(char const* path)
    {
        tinygltf::TinyGLTF context;

        tinygltf::Model model;
        std::string error, warning;
        if (!context.LoadASCIIFromFile(&model, &error, &warning, path))
        {
            // Log errors/warnings
            return false;
        }

        auto const& accessors = model.accessors;
        auto const& nodes = model.nodes;

        if (model.defaultScene > -1)
        {
            // There is a default scene, handle it
        }
        for (auto const& scene : model.scenes)
        {
            // Import scene
        }

        for (auto const& camera : model.cameras)
        {
            // Import camera
        }

        for (size_t meshIndex = 0; meshIndex < model.meshes.size(); ++meshIndex)
        {
            bhMesh_gltf newMesh;
            if (newMesh.Import(model, meshIndex))
            {
                //newMesh.Save();
            }
        }
        return true;
    }
}

#if 0
bhResourceIndex_t bhMesh::Load_gltf(char const* fileName)
{
    bhResourceIndex_t meshIndex = meshes.Find(fileName);
    if ((meshIndex != BH_INVALID_INDEX) && (meshes.GetResourceStatus(meshIndex) == bhResourceStatus::RS_READY))
    {
        return meshIndex;
    }
    meshIndex = meshes.GetNextFreeResourceIndex(fileName);
    if (meshIndex == BH_INVALID_INDEX) return BH_INVALID_INDEX;
    bhMesh* mesh = meshes.GetResource(meshIndex);

    std::string path(bhUtil::CreatePath(bhEnv::GetEnvString(bhEnv::MESHES_PATH), fileName));

    if (!tinygltf::FileExists(path, nullptr))
    {
        return BH_INVALID_INDEX;
    }
    tinygltf::TinyGLTF context;

    tinygltf::Model model;
    std::string error, warning;
    if (!context.LoadASCIIFromFile(&model, &error, &warning, path))
    {
        // Log errors/warnings
        return BH_INVALID_INDEX;
    }
    if (!mesh->Load_gltf(model))
    {
        return BH_INVALID_INDEX;
    }
    return meshIndex;
}
#endif
