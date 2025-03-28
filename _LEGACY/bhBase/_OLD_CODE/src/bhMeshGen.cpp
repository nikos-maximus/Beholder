#include <stdlib.h>
#include "bhMesh.hpp"

// All faces are defined in CCW order
// up is (0, 0, 1)

//================================================================================
// Plane
bhMesh* bhMesh_CreatePlane(float _sizex, float _sizey, int bindings)
{
	bhMesh* newMesh = bhMesh_New(4, 6, 1);
	newMesh->bindingsMask = bindings;

	_sizex *= 0.5f;
	_sizey *= 0.5f;

	if (newMesh->bindingsMask & MESH_BINDING_BIT_POSITIONS)
	{
		// xy plane, normal +z
		newMesh->verts[0].position = { -_sizex, -_sizey, 0.f };
		newMesh->verts[1].position = { _sizex, -_sizey, 0.f };
		newMesh->verts[2].position = { _sizex, _sizey, 0.f };
		newMesh->verts[3].position = { -_sizex, _sizey, 0.f };
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_NORMALS)
	{
		newMesh->verts[0].normal = newMesh->verts[1].normal = newMesh->verts[2].normal = newMesh->verts[3].normal = { 0.f, 0.f, 1.f };
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_UV_0)
	{
		for (short i = 0; i < 4; ++i)
		{
			newMesh->verts[i].uv.x = newMesh->verts[i].position.x;
			newMesh->verts[i].uv.y = newMesh->verts[i].position.y;
		}
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_TANGENTS)
	{
		newMesh->verts[0].tangent = newMesh->verts[1].tangent = newMesh->verts[2].tangent = newMesh->verts[3].tangent = { 1.f, 0.f, 0.f };
	}

	newMesh->inds[0] = 0;
	newMesh->inds[1] = 1;
	newMesh->inds[2] = 2;

	newMesh->inds[3] = 2;
	newMesh->inds[4] = 3;
	newMesh->inds[5] = 0;

	newMesh->primitives[0].vertexOffset = 0;
	newMesh->primitives[0].numVerts = newMesh->numVerts;
	newMesh->primitives[0].indexOffset = 0;
	newMesh->primitives[0].numInds = newMesh->numInds;

	return newMesh;
}

bhMesh* bhMesh_CreatePlane(glm::vec2 const& size, int bindings)
{
	return bhMesh_CreatePlane(size.x, size.y, bindings);
}

//================================================================================
// Box
bhMesh* bhMesh_CreateBox(float _sizex, float _sizey, float _sizez, int bindings)
{
	bhMesh* newMesh = bhMesh_New(24, 36, 1);
	newMesh->bindingsMask = bindings;

	_sizex *= 0.5f;
	_sizey *= 0.5f;
	_sizez *= 0.5f;

	if (newMesh->bindingsMask & MESH_BINDING_BIT_POSITIONS)
	{
		// yz plane, normal +x
		newMesh->verts[0].position = { _sizex, -_sizey, -_sizez };
		newMesh->verts[1].position = { _sizex, _sizey, -_sizez };
		newMesh->verts[2].position = { _sizex, _sizey, _sizez };
		newMesh->verts[3].position = { _sizex, -_sizey, _sizez };

		// yz plane, normal -x
		newMesh->verts[4].position = { -_sizex, _sizey, -_sizez };
		newMesh->verts[5].position = { -_sizex, -_sizey, -_sizez };
		newMesh->verts[6].position = { -_sizex, -_sizey, _sizez };
		newMesh->verts[7].position = { -_sizex, _sizey, _sizez };

		// xz plane, normal +y
		newMesh->verts[8].position = { _sizex, _sizey, -_sizez };
		newMesh->verts[9].position = { -_sizex, _sizey, -_sizez };
		newMesh->verts[10].position = { -_sizex, _sizey, _sizez };
		newMesh->verts[11].position = { _sizex, _sizey, _sizez };

		// xz plane, normal -y
		newMesh->verts[12].position = { -_sizex, -_sizey, -_sizez };
		newMesh->verts[13].position = { _sizex, -_sizey, -_sizez };
		newMesh->verts[14].position = { _sizex, -_sizey, _sizez };
		newMesh->verts[15].position = { -_sizex, -_sizey, _sizez };

		// xy plane, normal +z
		newMesh->verts[16].position = { -_sizex, -_sizey, _sizez };
		newMesh->verts[17].position = { _sizex, -_sizey, _sizez };
		newMesh->verts[18].position = { _sizex, _sizey, _sizez };
		newMesh->verts[19].position = { -_sizex, _sizey, _sizez };

		// xy plane, normal -z
		newMesh->verts[20].position = { _sizex, -_sizey, -_sizez };
		newMesh->verts[21].position = { -_sizex, -_sizey, -_sizez };
		newMesh->verts[22].position = { -_sizex, _sizey, -_sizez };
		newMesh->verts[23].position = { _sizex, _sizey, -_sizez };
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_NORMALS)
	{
		newMesh->verts[0].normal = newMesh->verts[1].normal = newMesh->verts[2].normal = newMesh->verts[3].normal = { 1.f, 0.f, 0.f };
		newMesh->verts[4].normal = newMesh->verts[5].normal = newMesh->verts[6].normal = newMesh->verts[7].normal = { -1.f, 0.f, 0.f };
		newMesh->verts[8].normal = newMesh->verts[9].normal = newMesh->verts[10].normal = newMesh->verts[11].normal = { 0.f, 1.f, 0.f };
		newMesh->verts[12].normal = newMesh->verts[13].normal = newMesh->verts[14].normal = newMesh->verts[15].normal = { 0.f, -1.f, 0.f };
		newMesh->verts[16].normal = newMesh->verts[17].normal = newMesh->verts[18].normal = newMesh->verts[19].normal = { 0.f, 0.f, 1.f };
		newMesh->verts[20].normal = newMesh->verts[21].normal = newMesh->verts[22].normal = newMesh->verts[23].normal = { 0.f, 0.f, -1.f };
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_UV_0)
	{
		for (short i = 0; i < 4; ++i)
		{
			short offs = i;
			newMesh->verts[offs].uv.x = newMesh->verts[offs].position.y;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;

			offs += 4;
			newMesh->verts[offs].uv.x = -newMesh->verts[offs].position.y;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;

			offs += 4;
			newMesh->verts[offs].uv.x = -newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;

			offs += 4;
			newMesh->verts[offs].uv.x = newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;

			offs += 4;
			newMesh->verts[offs].uv.x = newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.y;

			offs += 4;
			newMesh->verts[offs].uv.x = -newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.y;
		}
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_TANGENTS)
	{
		newMesh->verts[0].tangent = newMesh->verts[1].tangent = newMesh->verts[2].tangent = newMesh->verts[3].tangent = { 0.f, 1.f, 0.f };
		newMesh->verts[4].tangent = newMesh->verts[5].tangent = newMesh->verts[6].tangent = newMesh->verts[7].tangent = { 0.f, -1.f, 0.f };
		newMesh->verts[8].tangent = newMesh->verts[9].tangent = newMesh->verts[10].tangent = newMesh->verts[11].tangent = { -1.f, 0.f, 0.f };
		newMesh->verts[12].tangent = newMesh->verts[13].tangent = newMesh->verts[14].tangent = newMesh->verts[15].tangent = { 1.f, 0.f, 0.f };
		newMesh->verts[16].tangent = newMesh->verts[17].tangent = newMesh->verts[18].tangent = newMesh->verts[19].tangent = { 1.f, 0.f, 0.f };
		newMesh->verts[20].tangent = newMesh->verts[21].tangent = newMesh->verts[22].tangent = newMesh->verts[23].tangent = { -1.f, 0.f, 0.f };
	}

	int baseindex = 0;
	int baseVertex = 0;
	for (short f = 0; f < 6; ++f)
	{
		newMesh->inds[baseindex + 0] = baseVertex + 0;
		newMesh->inds[baseindex + 1] = baseVertex + 1;
		newMesh->inds[baseindex + 2] = baseVertex + 2;

		newMesh->inds[baseindex + 3] = baseVertex + 2;
		newMesh->inds[baseindex + 4] = baseVertex + 3;
		newMesh->inds[baseindex + 5] = baseVertex + 0;
		baseindex += 6;
		baseVertex += 4;
	}

	newMesh->primitives[0].vertexOffset = 0;
	newMesh->primitives[0].numVerts = newMesh->numVerts;
	newMesh->primitives[0].indexOffset = 0;
	newMesh->primitives[0].numInds = newMesh->numInds;

	return newMesh;
}

bhMesh* bhMesh_CreateBox(glm::vec3 const& size, int bindings)
{
	return bhMesh_CreateBox(size.x, size.y, size.z, bindings);
}

//================================================================================
// DungeonBlock - no floor/ceil
bhMesh* bhMesh_CreateBoxNoTopBottom(float _sizex, float _sizey, float _sizez, int bindings)
{
	bhMesh* newMesh = bhMesh_New(16, 24, 1);
	newMesh->bindingsMask = bindings;

	_sizex *= 0.5f;
	_sizey *= 0.5f;
	_sizez *= 0.5f;

	if (newMesh->bindingsMask & MESH_BINDING_BIT_POSITIONS)
	{
		// yz plane, normal +x
		newMesh->verts[0].position = { _sizex, -_sizey, -_sizez };
		newMesh->verts[1].position = { _sizex, _sizey, -_sizez };
		newMesh->verts[2].position = { _sizex, _sizey, _sizez };
		newMesh->verts[3].position = { _sizex, -_sizey, _sizez };

		// yz plane, normal -x
		newMesh->verts[4].position = { -_sizex, _sizey, -_sizez };
		newMesh->verts[5].position = { -_sizex, -_sizey, -_sizez };
		newMesh->verts[6].position = { -_sizex, -_sizey, _sizez };
		newMesh->verts[7].position = { -_sizex, _sizey, _sizez };

		// xz plane, normal +y
		newMesh->verts[8].position = { _sizex, _sizey, -_sizez };
		newMesh->verts[9].position = { -_sizex, _sizey, -_sizez };
		newMesh->verts[10].position = { -_sizex, _sizey, _sizez };
		newMesh->verts[11].position = { _sizex, _sizey, _sizez };

		// xz plane, normal -y
		newMesh->verts[12].position = { -_sizex, -_sizey, -_sizez };
		newMesh->verts[13].position = { _sizex, -_sizey, -_sizez };
		newMesh->verts[14].position = { _sizex, -_sizey, _sizez };
		newMesh->verts[15].position = { -_sizex, -_sizey, _sizez };
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_NORMALS)
	{
		newMesh->verts[0].normal = newMesh->verts[1].normal = newMesh->verts[2].normal = newMesh->verts[3].normal = { 1.f, 0.f, 0.f };
		newMesh->verts[4].normal = newMesh->verts[5].normal = newMesh->verts[6].normal = newMesh->verts[7].normal = { -1.f, 0.f, 0.f };
		newMesh->verts[8].normal = newMesh->verts[9].normal = newMesh->verts[10].normal = newMesh->verts[11].normal = { 0.f, 1.f, 0.f };
		newMesh->verts[12].normal = newMesh->verts[13].normal = newMesh->verts[14].normal = newMesh->verts[15].normal = { 0.f, -1.f, 0.f };
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_UV_0)
	{
		for (short i = 0; i < 4; ++i)
		{
			short offs = i;
			newMesh->verts[offs].uv.x = newMesh->verts[offs].position.y;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;

			offs += 4;
			newMesh->verts[offs].uv.x = -newMesh->verts[offs].position.y;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;

			offs += 4;
			newMesh->verts[offs].uv.x = -newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;

			offs += 4;
			newMesh->verts[offs].uv.x = newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.z;
		}
	}
	if (newMesh->bindingsMask & MESH_BINDING_BIT_TANGENTS)
	{
		newMesh->verts[0].tangent = newMesh->verts[1].tangent = newMesh->verts[2].tangent = newMesh->verts[3].tangent = { 0.f, 1.f, 0.f };
		newMesh->verts[4].tangent = newMesh->verts[5].tangent = newMesh->verts[6].tangent = newMesh->verts[7].tangent = { 0.f, -1.f, 0.f };
		newMesh->verts[8].tangent = newMesh->verts[9].tangent = newMesh->verts[10].tangent = newMesh->verts[11].tangent = { -1.f, 0.f, 0.f };
		newMesh->verts[12].tangent = newMesh->verts[13].tangent = newMesh->verts[14].tangent = newMesh->verts[15].tangent = { 1.f, 0.f, 0.f };
	}

	int baseindex = 0;
	int baseVertex = 0;
	for (short f = 0; f < 4; ++f)
	{
		newMesh->inds[baseindex + 0] = baseVertex + 0;
		newMesh->inds[baseindex + 1] = baseVertex + 1;
		newMesh->inds[baseindex + 2] = baseVertex + 2;

		newMesh->inds[baseindex + 3] = baseVertex + 2;
		newMesh->inds[baseindex + 4] = baseVertex + 3;
		newMesh->inds[baseindex + 5] = baseVertex + 0;
		baseindex += 6;
		baseVertex += 4;
	}

	newMesh->primitives[0].vertexOffset = 0;
	newMesh->primitives[0].numVerts = newMesh->numVerts;
	newMesh->primitives[0].indexOffset = 0;
	newMesh->primitives[0].numInds = newMesh->numInds;

	return newMesh;
}

bhMesh* bhMesh_CreateBoxNoTopBottom(glm::vec3 const& size, int bindings)
{
	return bhMesh_CreateBoxNoTopBottom(size.x, size.y, size.z, bindings);
}

#if MESH_GEN_OBSOLETE

//================================================================================
// DungeonFloorCeil - empty block from the inside
bhMesh* bhMesh_CreateDungeonFloorCeil(float _sizex, float _sizey, float _sizez, int bindings)
{
	bhMesh* newMesh = bhMesh_New(8, 12, 1);
	newMesh->bindingsMask = bindings;

	_sizex *= 0.5f;
	_sizey *= 0.5f;
	_sizez *= 0.5f;

	if (newMesh->bindingsMask & POSITIONS_BIT)
	{
		// xy plane, normal +z - floor
		newMesh->verts[0].position = { -_sizex, -_sizey, -_sizez };
		newMesh->verts[1].position = { _sizex, -_sizey, -_sizez };
		newMesh->verts[2].position = { _sizex, _sizey, -_sizez };
		newMesh->verts[3].position = { -_sizex, _sizey, -_sizez };

		// xy plane, normal -z - ceiling
		newMesh->verts[4].position = { _sizex, -_sizey, _sizez };
		newMesh->verts[5].position = { -_sizex, -_sizey, _sizez };
		newMesh->verts[6].position = { -_sizex, _sizey, _sizez };
		newMesh->verts[7].position = { _sizex, _sizey, _sizez };
	}
	if (newMesh->bindingsMask & NORMALS_BIT)
	{
		newMesh->verts[0].normal = newMesh->verts[1].normal = newMesh->verts[2].normal = newMesh->verts[3].normal = { 0.f, 0.f, 1.f };
		newMesh->verts[4].normal = newMesh->verts[5].normal = newMesh->verts[6].normal = newMesh->verts[7].normal = { 0.f, 0.f, -1.f };
	}
	if (newMesh->bindingsMask & UV_0_BIT)
	{
		for (short i = 0; i < 4; ++i)
		{
			short offs = i;
			newMesh->verts[offs].uv.x = newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.y;

			offs += 4;
			newMesh->verts[offs].uv.x = -newMesh->verts[offs].position.x;
			newMesh->verts[offs].uv.y = newMesh->verts[offs].position.y;
		}
	}
	if (newMesh->bindingsMask & TANGENTS_BIT)
	{
		newMesh->verts[0].tangent = newMesh->verts[1].tangent = newMesh->verts[2].tangent = newMesh->verts[3].tangent = { 1.f, 0.f, 0.f };
		newMesh->verts[4].tangent = newMesh->verts[5].tangent = newMesh->verts[6].tangent = newMesh->verts[7].tangent = { -1.f, 0.f, 0.f };
	}

	int baseindex = 0;
	int baseVertex = 0;
	for (short f = 0; f < 2; ++f)
	{
		newMesh->inds[baseindex + 0] = baseVertex + 0;
		newMesh->inds[baseindex + 1] = baseVertex + 1;
		newMesh->inds[baseindex + 2] = baseVertex + 2;

		newMesh->inds[baseindex + 3] = baseVertex + 2;
		newMesh->inds[baseindex + 4] = baseVertex + 3;
		newMesh->inds[baseindex + 5] = baseVertex + 0;
		baseindex += 6;
		baseVertex += 4;
	}

	newMesh->primitives[0].vertexOffset = 0;
	newMesh->primitives[0].numVerts = newMesh->numVerts;
	newMesh->primitives[0].indexOffset = 0;
	newMesh->primitives[0].numInds = newMesh->numInds;

	return newMesh;
}

bhMesh* bhMesh_CreateDungeonFloorCeil(glm::vec3 const& size, int bindings)
{
	return bhMesh_CreateDungeonFloorCeil(size.x, size.y, size.z, bindings);
}

#endif
