#define TINYGLTF_IMPLEMENTATION
//#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "bhImport.hpp"
#include "bhMesh.hpp"
#include "bhLog.h"

namespace bhImport
{
	static constexpr size_t STRING_BUF_SIZ = 256;
	static char g_stringBuf[STRING_BUF_SIZ];
	static char const* meshExtension = ".bhm";

	bhMesh* bhMesh_Import_tinygltf(const tinygltf::Mesh& mesh, const tinygltf::Model& gltfModel)
	{
		struct DataPerPrimitive
		{
			const float* positionBuffer { nullptr };
			const float* normalsBuffer { nullptr };
			const float* tangentsBuffer { nullptr };
			const float* texCoordsBuffer_0 { nullptr };
			const void* indexBuffer{ nullptr };
			uint32_t numVerts { 0 };
			uint32_t numInds { 0 };
			int indexType{ 0 };
			int bindingsMask { 0 };
		};

		uint32_t numMeshVerts = 0;
		uint32_t numMeshInds = 0;

		std::vector<DataPerPrimitive> dataPerPrimitive(mesh.primitives.size());

		for (size_t primitiveIdx = 0; primitiveIdx < mesh.primitives.size(); ++primitiveIdx)
		{
			const tinygltf::Primitive& gltfPrimitive = mesh.primitives[primitiveIdx];
			{
				// Vertex positions
				auto positionsAttr = gltfPrimitive.attributes.find("POSITION");
				if (positionsAttr != gltfPrimitive.attributes.end())
				{
					dataPerPrimitive[primitiveIdx].bindingsMask |= MESH_BINDING_BIT_POSITIONS;

					const tinygltf::Accessor& accessor = gltfModel.accessors[positionsAttr->second];
					const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
					dataPerPrimitive[primitiveIdx].positionBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					dataPerPrimitive[primitiveIdx].numVerts = accessor.count;

					numMeshVerts += dataPerPrimitive[primitiveIdx].numVerts;
				}
			}
			{
				// Normals
				auto normalsAttr = gltfPrimitive.attributes.find("NORMAL");
				if (normalsAttr != gltfPrimitive.attributes.end())
				{
					dataPerPrimitive[primitiveIdx].bindingsMask |= MESH_BINDING_BIT_NORMALS;

					const tinygltf::Accessor& accessor = gltfModel.accessors[normalsAttr->second];
					const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
					dataPerPrimitive[primitiveIdx].normalsBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
			}
			{
				// Tangents
				auto tangentAttr = gltfPrimitive.attributes.find("TANGENT");
				if (tangentAttr != gltfPrimitive.attributes.end())
				{
					dataPerPrimitive[primitiveIdx].bindingsMask |= MESH_BINDING_BIT_TANGENTS;

					const tinygltf::Accessor& accessor = gltfModel.accessors[tangentAttr->second];
					const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
					dataPerPrimitive[primitiveIdx].tangentsBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
			}
			{
				// TexCoord_0
				auto texCoordAttr_0 = gltfPrimitive.attributes.find("TEXCOORD_0");
				if (texCoordAttr_0 != gltfPrimitive.attributes.end())
				{
					dataPerPrimitive[primitiveIdx].bindingsMask |= MESH_BINDING_BIT_UV_0;

					const tinygltf::Accessor& accessor = gltfModel.accessors[texCoordAttr_0->second];
					const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
					dataPerPrimitive[primitiveIdx].texCoordsBuffer_0 = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
			}

			{
				// Indexes
				const tinygltf::Accessor& accessor = gltfModel.accessors[gltfPrimitive.indices];
				const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = gltfModel.buffers[view.buffer];

				dataPerPrimitive[primitiveIdx].numInds = accessor.count;
				numMeshInds += dataPerPrimitive[primitiveIdx].numInds;

				dataPerPrimitive[primitiveIdx].indexBuffer = &(buffer.data[accessor.byteOffset + view.byteOffset]);
				dataPerPrimitive[primitiveIdx].indexType = accessor.componentType;
				//switch (accessor.componentType)
				//{
				//	case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				//	{
				//	#if BH_USE_MESH_INDEX_TYPE_UNIT16
				//		assert(false);
				//		break;
				//	#endif // BH_USE_MESH_INDEX_TYPE_UNIT16
				//		break;
				//	}
				//	case TINYGLTF_PARAMETER_TYPE_SHORT:
				//	{
				//		break;
				//	}
				//	case TINYGLTF_PARAMETER_TYPE_BYTE:
				//	{
				//		break;
				//	}
				//}
			}

		}

		bhMesh* newMesh = bhMesh_New(numMeshVerts, numMeshInds, dataPerPrimitive.size());
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;
		for (size_t meshPrimitiveIdx = 0; meshPrimitiveIdx < dataPerPrimitive.size(); ++meshPrimitiveIdx)
		{
			newMesh->primitives[meshPrimitiveIdx].vertexOffset = vertexOffset;
			newMesh->primitives[meshPrimitiveIdx].numVerts = dataPerPrimitive[meshPrimitiveIdx].numVerts;

			const float* primitivePositions = dataPerPrimitive[meshPrimitiveIdx].positionBuffer;
			const float* primitiveNormals = dataPerPrimitive[meshPrimitiveIdx].normalsBuffer;
			const float* primitiveTangents = dataPerPrimitive[meshPrimitiveIdx].tangentsBuffer;
			const float* primitiveTexCoords_0 = dataPerPrimitive[meshPrimitiveIdx].texCoordsBuffer_0;
			for (uint32_t vIdx = 0; vIdx < newMesh->primitives[meshPrimitiveIdx].numVerts; ++vIdx)
			{
				if (primitivePositions != nullptr)
				{
					newMesh->verts[vertexOffset + vIdx].position.x = *primitivePositions++;
					newMesh->verts[vertexOffset + vIdx].position.y = *primitivePositions++;
					newMesh->verts[vertexOffset + vIdx].position.z = *primitivePositions++;
				}
				if (primitiveNormals != nullptr)
				{
					newMesh->verts[vertexOffset + vIdx].normal.x = *primitiveNormals++;
					newMesh->verts[vertexOffset + vIdx].normal.y = *primitiveNormals++;
					newMesh->verts[vertexOffset + vIdx].normal.z = *primitiveNormals++;
				}
				if (primitiveTangents != nullptr)
				{
					newMesh->verts[vertexOffset + vIdx].tangent.x = *primitiveTangents++;
					newMesh->verts[vertexOffset + vIdx].tangent.y = *primitiveTangents++;
					newMesh->verts[vertexOffset + vIdx].tangent.z = *primitiveTangents++;
				}
				if (primitiveTexCoords_0 != nullptr)
				{
					newMesh->verts[vertexOffset + vIdx].uv.x = *primitiveTexCoords_0++;
					newMesh->verts[vertexOffset + vIdx].uv.y = *primitiveTexCoords_0++;
				}
			}
			vertexOffset += dataPerPrimitive[meshPrimitiveIdx].numVerts;

			newMesh->primitives[meshPrimitiveIdx].indexOffset = indexOffset;
			newMesh->primitives[meshPrimitiveIdx].numInds = dataPerPrimitive[meshPrimitiveIdx].numInds;

			switch (dataPerPrimitive[meshPrimitiveIdx].indexType)
			{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
				#if BH_USE_MESH_INDEX_TYPE_UNIT16
					assert(false);
					break;
				#endif // BH_USE_MESH_INDEX_TYPE_UNIT16

					uint32_t* buf = (uint32_t*)dataPerPrimitive[meshPrimitiveIdx].indexBuffer;
					for (uint32_t iIdx = 0; iIdx < newMesh->primitives[meshPrimitiveIdx].numInds; ++iIdx)
					{
						newMesh->inds[indexOffset + iIdx] = buf[iIdx];
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					uint16_t* buf = (uint16_t*)dataPerPrimitive[meshPrimitiveIdx].indexBuffer;
					for (uint32_t iIdx = 0; iIdx < newMesh->primitives[meshPrimitiveIdx].numInds; ++iIdx)
					{
						newMesh->inds[indexOffset + iIdx] = buf[iIdx];
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_SHORT:
				{
					int16_t* buf = (int16_t*)dataPerPrimitive[meshPrimitiveIdx].indexBuffer;
					for (uint32_t iIdx = 0; iIdx < newMesh->primitives[meshPrimitiveIdx].numInds; ++iIdx)
					{
						newMesh->inds[indexOffset + iIdx] = buf[iIdx];
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_BYTE:
				{
					int8_t* buf = (int8_t*)dataPerPrimitive[meshPrimitiveIdx].indexBuffer;
					for (uint32_t iIdx = 0; iIdx < newMesh->primitives[meshPrimitiveIdx].numInds; ++iIdx)
					{
						newMesh->inds[indexOffset + iIdx] = buf[iIdx];
					}
					break;
				}
			}
			indexOffset += dataPerPrimitive[meshPrimitiveIdx].numInds;
		}

		// TODO: DEBUG
		// TODO: DEBUG
		newMesh->bindingsMask = dataPerPrimitive[0].bindingsMask;
		// TODO: DEBUG

		bhLog_Message(LOG_TYPE_INFO, "Imported mesh %s, %d verts, %d inds.\n\tNormals : %s\n\tUV : %s\n\tTangents : %s\n",
			mesh.name.c_str(),
			numMeshVerts, numMeshInds,
			newMesh->bindingsMask & MESH_BINDING_BIT_NORMALS ? "Yes" : "No",
			newMesh->bindingsMask& MESH_BINDING_BIT_UV_0 ? "Yes" : "No",
			newMesh->bindingsMask& MESH_BINDING_BIT_TANGENTS ? "Yes" : "No"
		);

		return newMesh;
	}

	bool ImportScene(char const* filePath)
	{
		tinygltf::TinyGLTF gltfContext;
		tinygltf::Model gltfModel;
		std::string error, warning;

		bool loadResult = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filePath);
		if (!loadResult)
		{
			return false;
		}

		//for (size_t sceneIdx = 0; sceneIdx < gltfModel.scenes.size(); ++sceneIdx)
		//{
		//	const tinygltf::Scene& scene = gltfModel.scenes[sceneIdx];
		//	for (size_t nodeIdx = 0; nodeIdx < scene.nodes.size(); ++nodeIdx)
		//	{
		//		const tinygltf::Node& node = gltfModel.nodes[scene.nodes[nodeIdx]];
		//		bhMesh_Editor mesh;
		//		mesh.Import_tinygltf(node);
		//		sprintf_s(g_stringBuf, STRING_BUF_SIZ, "%s.%s", node.name.c_str(), meshExtension);
		//		mesh.Save(g_stringBuf);
		//	}
		//}

		if (gltfModel.animations.size() > 0)
		{
		}

		if (gltfModel.cameras.size() > 0)
		{
		}

		if (gltfModel.lights.size() > 0)
		{
			//for (unsigned int i = 0; i < scene->mNumLights; ++i)
			//{
			//	scene->mLights[i]->mType
			//}
		}

		if (gltfModel.materials.size() > 0)
		{
		}

		if (gltfModel.meshes.size() > 0)
		{
			for (auto& gltfMesh : gltfModel.meshes)
			{
				bhMesh* newMesh = bhMesh_Import_tinygltf(gltfMesh, gltfModel);
				sprintf_s(g_stringBuf, STRING_BUF_SIZ, "%s.%s", gltfMesh.name.c_str(), meshExtension);
				bhMesh_Save(g_stringBuf, newMesh);
			}
		}

		if (gltfModel.textures.size() > 0)
		{
		}

		return true;
	}
}
