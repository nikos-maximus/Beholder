#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Game/bhWorld.hpp"
#include "Mesh/bhMesh.hpp"
#include "Platform/bhPlatform.hpp"

bool HandleNode(aiNode* node)
{
	if (!node) return false;
	for (unsigned int mIdx = 0; mIdx < node->mNumMeshes; ++mIdx)
	{
		//Handle mesh indexes
		//workingNode->mMeshes[mIdx];
	}
	for (unsigned int cIdx = 0; cIdx < node->mNumChildren; ++cIdx)
	{
		return HandleNode(node->mChildren[cIdx]);
	}
	return true;
}

bool bhWorld::Import(const char* filePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath,
		aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);

	if (!scene)
	{
		// TODO: Log error
		return false;
	}

	// Import meshes
	for (unsigned int mIdx = 0; mIdx < scene->mNumMeshes; ++mIdx)
	{
		aiMesh* aMesh = scene->mMeshes[mIdx];
		bhMesh bmesh;
		if (bmesh.ImportAssimp(aMesh))
		{
			bmesh.Save(aMesh->mName.C_Str());
		}
	}

	// Import scene
	//HandleNode(scene->mRootNode);

	return true;
}
