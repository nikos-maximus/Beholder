#include "bhTypes.hpp"
#include "Mesh/bhMesh.hpp"
#include "Platform/bhPlatform.hpp"
#include "bhLog.h"

////////////////////////////////////////////////////////////////////////////////
void bhMesh::_Allocate(uint32_t _numVerts, uint32_t _numInds)
{
	numVerts = _numVerts;
	verts = (bhMeshVertex*)calloc(numVerts, sizeof(bhMeshVertex));

	numInds = _numInds;
	inds = (bhMeshIdx_t*)calloc(numInds, sizeof(bhMeshIdx_t));

	deviceData = {};
}

void bhMesh::_Free()
{
	free(inds);
	inds = nullptr;

	free(verts);
	verts = nullptr;
}

bhMesh::bhMesh()
	: verts(nullptr)
	, inds(nullptr)
	, numVerts(0)
	, numInds(0)
{
	deviceData = {};
}

bhMesh::bhMesh(uint32_t _numVerts, uint32_t _numInds)
{
	_Allocate(_numVerts, _numInds);
}

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

	setup.SetMask(mask);
	_Allocate(numVerts, numInds);
	fread(verts, sizeof(bhMeshVertex), numVerts, file);
	fread(inds, sizeof(bhMeshIdx_t), numInds, file);

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

	int mask = setup.GetMask();
	fwrite(&mask, sizeof(mask), 1, file);
	fwrite(&numVerts, sizeof(numVerts), 1, file);
	fwrite(&numInds, sizeof(numInds), 1, file);

	fwrite(verts, sizeof(bhMeshVertex), numVerts, file);
	fwrite(inds, sizeof(bhMeshIdx_t), numInds, file);

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

////////////////////////////////////////////////////////////////////////////////
#if 0
void bhMesh::CalcBoundingBox()
{
	glm::vec3 minBound, maxBound;
	minBound = maxBound = verts[0].position;

	for (uint32_t v = 1; v < numVerts; ++v)
	{
		auto& cv = verts[v];
		if (cv.position.x < minBound.x)
		{
			minBound.x = cv.position.x;
		}
		else
		{
			maxBound.x = cv.position.x;
		}
		if (cv.position.y < minBound.y)
		{
			minBound.y = cv.position.y;
		}
		else
		{
			maxBound.y = cv.position.y;
		}
		if (cv.position.z < minBound.z)
		{
			minBound.z = cv.position.z;
		}
		else
		{
			maxBound.z = cv.position.z;
		}
	}
	bhBoxf bbox(maxBound - minBound);
	//bbox.center = minBound + bbox.halfSiz;
}
#endif
