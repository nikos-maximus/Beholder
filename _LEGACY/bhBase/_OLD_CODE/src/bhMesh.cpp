#include "bhMesh.hpp"
#include "bhUtil.h"
#include "bhEnv.h"
#include "bhLog.h"
//#include "bhBox.h"

////////////////////////////////////////////////////////////////////////////////
bhMesh* bhMesh_New(uint32_t _numVerts, uint32_t _numInds, uint32_t numPrimitives)
{
	bhMesh* newMesh = new bhMesh();

	newMesh->numVerts = _numVerts;
	newMesh->numInds = _numInds;
	newMesh->numPrimitives = numPrimitives;

	newMesh->verts = (bhWorldVertex*)calloc(newMesh->numVerts, sizeof(bhWorldVertex));
	newMesh->inds = (bhMeshIdx_t*)calloc(newMesh->numInds, sizeof(bhMeshIdx_t));
	newMesh->primitives = (bhMesh::Primitive*)calloc(newMesh->numPrimitives, sizeof(bhMesh::Primitive));

	return newMesh;
}

void bhMesh_Delete(bhMesh** mesh)
{
	free((*mesh)->primitives);
	(*mesh)->primitives = nullptr;

	free((*mesh)->inds);
	(*mesh)->inds = nullptr;

	free((*mesh)->verts);
	(*mesh)->verts = nullptr;

	delete *mesh;
	*mesh = nullptr;
}

bhMesh* bhMesh_Load(FILE* file)
{
	bhMesh* newMesh = new bhMesh();

	fread(&(newMesh->bindingsMask), sizeof(newMesh->bindingsMask), 1, file);
	fread(&(newMesh->numVerts), sizeof(newMesh->numVerts), 1, file);
	fread(&(newMesh->numInds), sizeof(newMesh->numInds), 1, file);
	fread(&(newMesh->numPrimitives), sizeof(newMesh->numPrimitives), 1, file);

	newMesh->verts = (bhWorldVertex*)calloc(newMesh->numVerts, sizeof(bhWorldVertex));
	fread(newMesh->verts, sizeof(bhWorldVertex), newMesh->numVerts, file);

	newMesh->inds = (bhMeshIdx_t*)calloc(newMesh->numInds, sizeof(bhMeshIdx_t));
	fread(newMesh->inds, sizeof(bhMeshIdx_t), newMesh->numInds, file);

	newMesh->primitives = (bhMesh::Primitive*)calloc(newMesh->numPrimitives, sizeof(bhMesh::Primitive));
	fread(newMesh->primitives, sizeof(bhMesh::Primitive), newMesh->numPrimitives, file);

	// TODO: Should error-check the reads
	return newMesh;
}

bhMesh* bhMesh_Load(char const* fileName)
{
	char* path = bhUtil_CreatePath(bhEnv_GetEnvString(ENV_MESHES_PATH), fileName);
	FILE* file = nullptr;
	int openResult = fopen_s(&file, path, "rb");
	bhUtil_FreePath(&path);
	if (openResult == 0)
	{
		bhMesh* newMesh = bhMesh_Load(file);
		fclose(file);
		return newMesh;
	}
	bhLog_Message(LOG_TYPE_ERROR, "bhMesh_Load : file %s not found", path);
	return nullptr;
}

bool bhMesh_Save(FILE* file, const bhMesh* mesh)
{
	fwrite(&(mesh->bindingsMask), sizeof(mesh->bindingsMask), 1, file);
	fwrite(&(mesh->numVerts), sizeof(mesh->numVerts), 1, file);
	fwrite(&(mesh->numInds), sizeof(mesh->numInds), 1, file);
	fwrite(&(mesh->numPrimitives), sizeof(mesh->numPrimitives), 1, file);

	fwrite(mesh->verts, sizeof(bhWorldVertex), mesh->numVerts, file);
	fwrite(mesh->inds, sizeof(bhMeshIdx_t), mesh->numInds, file);
	fwrite(mesh->primitives, sizeof(bhMesh::Primitive), mesh->numPrimitives, file);
	
	return true; // TODO: Should error-check the writes
}

bool bhMesh_Save(char const* fileName, const bhMesh* mesh)
{
	char* path = bhUtil_CreatePath(bhEnv_GetEnvString(ENV_MESHES_PATH), fileName);
	FILE* file = nullptr;
	int openResult = fopen_s(&file, path, "wb");
	bhUtil_FreePath(&path);
	if (openResult == 0)
	{
		bool saveResult = bhMesh_Save(file, mesh);
		fclose(file);
		return saveResult;
	}
	bhLog_Message(LOG_TYPE_ERROR, "bhMesh::Save : could not open file %s for saving", path);
	return false;
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

void bhMesh_ApplyVertexOffset(bhMesh* mesh, float xOffs, float yOffs, float zOffs)
{
	for (uint32_t v = 0; v < mesh->numVerts; ++v)
	{
		mesh->verts[v].position.x = xOffs;
		mesh->verts[v].position.y = yOffs;
		mesh->verts[v].position.z = zOffs;
	}
}

void bhMesh_ApplyVertexOffset(bhMesh* mesh, glm::vec3 offs)
{
	bhMesh_ApplyVertexOffset(mesh, offs.x, offs.y, offs.z);
}

void bhMesh_ScaleUV(bhMesh* mesh, float xScale, float yScale)
{
	for (uint32_t v = 0; v < mesh->numVerts; ++v)
	{
		mesh->verts[v].uv.x *= xScale;
		mesh->verts[v].uv.y *= yScale;
	}
}

void bhMesh_ScaleUV(bhMesh* mesh, glm::vec2 scale)
{
	bhMesh_ScaleUV(mesh, scale.x, scale.y);
}

void bhMesh_ScaleUV(bhMesh* mesh, float uScale)
{
	bhMesh_ScaleUV(mesh, uScale, uScale);
}

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
