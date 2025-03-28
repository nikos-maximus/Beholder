#include <Mesh/bhMesh.hpp>
#include <assimp/mesh.h>

bool bhMesh::ImportAssimp(aiMesh* _aMesh)
{
	//Sanity checks
	bool isMeshComplete = _aMesh->HasPositions() && _aMesh->HasNormals() && _aMesh->HasTangentsAndBitangents() && _aMesh->HasTextureCoords(0);
	if (!isMeshComplete)
	{
		return false;
	}

	//We assume that all faces are triangles, see bhWorld::Import preprocessing flags
	_Allocate(_aMesh->mNumVertices, _aMesh->mNumFaces * 3);

	if (_aMesh->HasPositions()) //These checks are redundant, but let's keep them if we want to loose the restriction of isMeshComplete
	{
		setup.AddBinding(MESH_BINDING_POSITIONS);
		for (unsigned int vIdx = 0; vIdx < _aMesh->mNumVertices; ++vIdx)
		{
			verts[vIdx].position.x = _aMesh->mVertices[vIdx].x;
			verts[vIdx].position.y = _aMesh->mVertices[vIdx].y;
			verts[vIdx].position.z = _aMesh->mVertices[vIdx].z;
		}
	}
	if (_aMesh->HasNormals()) //These checks are redundant, but let's keep them if we want to loose the restriction of isMeshComplete
	{
		setup.AddBinding(MESH_BINDING_NORMALS);
		for (unsigned int nIdx = 0; nIdx < _aMesh->mNumVertices; ++nIdx)
		{
			verts[nIdx].normal.x = _aMesh->mNormals[nIdx].x;
			verts[nIdx].normal.y = _aMesh->mNormals[nIdx].y;
			verts[nIdx].normal.z = _aMesh->mNormals[nIdx].z;
		}
	}
	if (_aMesh->HasTangentsAndBitangents()) //These checks are redundant, but let's keep them if we want to loose the restriction of isMeshComplete
	{
		setup.AddBinding(MESH_BINDING_TANGENTS);
		for (unsigned int tIdx = 0; tIdx < _aMesh->mNumVertices; ++tIdx)
		{
			verts[tIdx].tangent.x = _aMesh->mTangents[tIdx].x;
			verts[tIdx].tangent.y = _aMesh->mTangents[tIdx].y;
			verts[tIdx].tangent.z = _aMesh->mTangents[tIdx].z;
		}
	}
	unsigned int numUVChannels = _aMesh->GetNumUVChannels();
	for (unsigned int channelIdx = 0; channelIdx < numUVChannels; ++channelIdx)
	{
		if (_aMesh->HasTextureCoords(channelIdx)) //These checks are redundant, but let's keep them if we want to loose the restriction of isMeshComplete
		{
			setup.AddBinding(MESH_BINDING_UV_0);
			for (unsigned int uvIdx = 0; uvIdx < _aMesh->mNumVertices; ++uvIdx)
			{
				verts[uvIdx].uv_0.x = _aMesh->mTextureCoords[channelIdx][uvIdx].x;
				verts[uvIdx].uv_0.y = _aMesh->mTextureCoords[channelIdx][uvIdx].y;
			}
		}
		if (channelIdx > 0) break; //Handle only one UV channel for now
	}

	size_t fi = 0;
	for (unsigned int faceIdx = 0; faceIdx < _aMesh->mNumFaces; ++faceIdx)
	{
		// TODO: Heavily relying on pre-triangulated mesh here. Generalize?
		aiFace& face = _aMesh->mFaces[faceIdx];
		assert(face.mNumIndices == 3);
		inds[fi++] = face.mIndices[0];
		inds[fi++] = face.mIndices[1];
		inds[fi++] = face.mIndices[2];
	}
	return true;
}
