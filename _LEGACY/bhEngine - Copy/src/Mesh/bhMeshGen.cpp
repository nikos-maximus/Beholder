#include <stdlib.h>
#include "bhTypes.hpp"
#include "Mesh/bhMesh.hpp"

// All faces are defined in CCW order
// up is (0, 0, 1)

//================================================================================
// Plane
void bhMesh::CreatePlane(float _sizex, float _sizey)
{
	_Allocate(4, 6);

	_sizex *= 0.5f;
	_sizey *= 0.5f;

	if(setup.HasBinding(MESH_BINDING_POSITIONS))
	{
		// xy plane, normal +z
		verts[0].position = { -_sizex, -_sizey, 0.0f };
		verts[1].position = { _sizex, -_sizey, 0.0f };
		verts[2].position = { _sizex, _sizey, 0.0f };
		verts[3].position = { -_sizex, _sizey, 0.0f };
	}
	if (setup.HasBinding(MESH_BINDING_NORMALS))
	{
		verts[0].normal = verts[1].normal = verts[2].normal = verts[3].normal = { 0.0f, 0.0f, 1.0f };
	}
	if (setup.HasBinding(MESH_BINDING_UV_0))
	{
		for (short i = 0; i < 4; ++i)
		{
			verts[i].uv_0.x = verts[i].position.x;
			verts[i].uv_0.y = verts[i].position.y;
		}
	}
	if (setup.HasBinding(MESH_BINDING_TANGENTS))
	{
		verts[0].tangent = verts[1].tangent = verts[2].tangent = verts[3].tangent = { 1.0f, 0.0f, 0.0f };
	}

	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;

	inds[3] = 2;
	inds[4] = 3;
	inds[5] = 0;
}

void bhMesh::CreatePlane(glm::vec2 const& size)
{
	return CreatePlane(size.x, size.y);
}

//================================================================================
// Box
void bhMesh::CreateBox(float _sizex, float _sizey, float _sizez)
{
	_Allocate(24, 36);

	_sizex *= 0.5f;
	_sizey *= 0.5f;
	_sizez *= 0.5f;

	if(setup.HasBinding(MESH_BINDING_POSITIONS))
	{
		// yz plane, normal +x
		verts[0].position = { _sizex, -_sizey, -_sizez };
		verts[1].position = { _sizex, _sizey, -_sizez };
		verts[2].position = { _sizex, _sizey, _sizez };
		verts[3].position = { _sizex, -_sizey, _sizez };

		// yz plane, normal -x
		verts[4].position = { -_sizex, _sizey, -_sizez };
		verts[5].position = { -_sizex, -_sizey, -_sizez };
		verts[6].position = { -_sizex, -_sizey, _sizez };
		verts[7].position = { -_sizex, _sizey, _sizez };

		// xz plane, normal +y
		verts[8].position = { _sizex, _sizey, -_sizez };
		verts[9].position = { -_sizex, _sizey, -_sizez };
		verts[10].position = { -_sizex, _sizey, _sizez };
		verts[11].position = { _sizex, _sizey, _sizez };

		// xz plane, normal -y
		verts[12].position = { -_sizex, -_sizey, -_sizez };
		verts[13].position = { _sizex, -_sizey, -_sizez };
		verts[14].position = { _sizex, -_sizey, _sizez };
		verts[15].position = { -_sizex, -_sizey, _sizez };

		// xy plane, normal +z
		verts[16].position = { -_sizex, -_sizey, _sizez };
		verts[17].position = { _sizex, -_sizey, _sizez };
		verts[18].position = { _sizex, _sizey, _sizez };
		verts[19].position = { -_sizex, _sizey, _sizez };

		// xy plane, normal -z
		verts[20].position = { _sizex, -_sizey, -_sizez };
		verts[21].position = { -_sizex, -_sizey, -_sizez };
		verts[22].position = { -_sizex, _sizey, -_sizez };
		verts[23].position = { _sizex, _sizey, -_sizez };
	}
	if (setup.HasBinding(MESH_BINDING_NORMALS))
	{
		verts[0].normal = verts[1].normal = verts[2].normal = verts[3].normal = { 1.0f, 0.0f, 0.0f };
		verts[4].normal = verts[5].normal = verts[6].normal = verts[7].normal = { -1.0f, 0.0f, 0.0f };
		verts[8].normal = verts[9].normal = verts[10].normal = verts[11].normal = { 0.0f, 1.0f, 0.0f };
		verts[12].normal = verts[13].normal = verts[14].normal = verts[15].normal = { 0.0f, -1.0f, 0.0f };
		verts[16].normal = verts[17].normal = verts[18].normal = verts[19].normal = { 0.0f, 0.0f, 1.0f };
		verts[20].normal = verts[21].normal = verts[22].normal = verts[23].normal = { 0.0f, 0.0f, -1.0f };
	}
	if (setup.HasBinding(MESH_BINDING_UV_0))
	{
		for (short i = 0; i < 4; ++i)
		{
			short offs = i;
			verts[offs].uv_0.x = verts[offs].position.y;
			verts[offs].uv_0.y = verts[offs].position.z;

			offs += 4;
			verts[offs].uv_0.x = -verts[offs].position.y;
			verts[offs].uv_0.y = verts[offs].position.z;

			offs += 4;
			verts[offs].uv_0.x = -verts[offs].position.x;
			verts[offs].uv_0.y = verts[offs].position.z;

			offs += 4;
			verts[offs].uv_0.x = verts[offs].position.x;
			verts[offs].uv_0.y = verts[offs].position.z;

			offs += 4;
			verts[offs].uv_0.x = verts[offs].position.x;
			verts[offs].uv_0.y = verts[offs].position.y;

			offs += 4;
			verts[offs].uv_0.x = -verts[offs].position.x;
			verts[offs].uv_0.y = verts[offs].position.y;
		}
	}
	if (setup.HasBinding(MESH_BINDING_TANGENTS))
	{
		verts[0].tangent = verts[1].tangent = verts[2].tangent = verts[3].tangent = { 0.0f, 1.0f, 0.0f };
		verts[4].tangent = verts[5].tangent = verts[6].tangent = verts[7].tangent = { 0.0f, -1.0f, 0.0f };
		verts[8].tangent = verts[9].tangent = verts[10].tangent = verts[11].tangent = { -1.0f, 0.0f, 0.0f };
		verts[12].tangent = verts[13].tangent = verts[14].tangent = verts[15].tangent = { 1.0f, 0.0f, 0.0f };
		verts[16].tangent = verts[17].tangent = verts[18].tangent = verts[19].tangent = { 1.0f, 0.0f, 0.0f };
		verts[20].tangent = verts[21].tangent = verts[22].tangent = verts[23].tangent = { -1.0f, 0.0f, 0.0f };
	}

	int baseindex = 0;
	int baseVertex = 0;
	for (short f = 0; f < 6; ++f)
	{
		inds[baseindex + 0] = baseVertex + 0;
		inds[baseindex + 1] = baseVertex + 1;
		inds[baseindex + 2] = baseVertex + 2;

		inds[baseindex + 3] = baseVertex + 2;
		inds[baseindex + 4] = baseVertex + 3;
		inds[baseindex + 5] = baseVertex + 0;
		baseindex += 6;
		baseVertex += 4;
	}
}

void bhMesh::CreateBox(glm::vec3 const& _size)
{
	return CreateBox(_size.x, _size.y, _size.z);
}

//================================================================================
// DungeonBlock - no floor/ceil
void bhMesh::CreateBoxSides(float _sizex, float _sizey, float _sizez)
{
	_Allocate(16, 24);

	_sizex *= 0.5f;
	_sizey *= 0.5f;
	_sizez *= 0.5f;

	if (setup.HasBinding(MESH_BINDING_POSITIONS))
	{
		// yz plane, normal +x
		verts[0].position = { _sizex, -_sizey, -_sizez };
		verts[1].position = { _sizex, _sizey, -_sizez };
		verts[2].position = { _sizex, _sizey, _sizez };
		verts[3].position = { _sizex, -_sizey, _sizez };

		// yz plane, normal -x
		verts[4].position = { -_sizex, _sizey, -_sizez };
		verts[5].position = { -_sizex, -_sizey, -_sizez };
		verts[6].position = { -_sizex, -_sizey, _sizez };
		verts[7].position = { -_sizex, _sizey, _sizez };

		// xz plane, normal +y
		verts[8].position = { _sizex, _sizey, -_sizez };
		verts[9].position = { -_sizex, _sizey, -_sizez };
		verts[10].position = { -_sizex, _sizey, _sizez };
		verts[11].position = { _sizex, _sizey, _sizez };

		// xz plane, normal -y
		verts[12].position = { -_sizex, -_sizey, -_sizez };
		verts[13].position = { _sizex, -_sizey, -_sizez };
		verts[14].position = { _sizex, -_sizey, _sizez };
		verts[15].position = { -_sizex, -_sizey, _sizez };
	}
	if (setup.HasBinding(MESH_BINDING_NORMALS))
	{
		verts[0].normal = verts[1].normal = verts[2].normal = verts[3].normal = { 1.0f, 0.0f, 0.0f };
		verts[4].normal = verts[5].normal = verts[6].normal = verts[7].normal = { -1.0f, 0.0f, 0.0f };
		verts[8].normal = verts[9].normal = verts[10].normal = verts[11].normal = { 0.0f, 1.0f, 0.0f };
		verts[12].normal = verts[13].normal = verts[14].normal = verts[15].normal = { 0.0f, -1.0f, 0.0f };
	}
	if (setup.HasBinding(MESH_BINDING_UV_0))
	{
		for (short i = 0; i < 4; ++i)
		{
			short offs = i;
			verts[offs].uv_0.x = verts[offs].position.y;
			verts[offs].uv_0.y = verts[offs].position.z;

			offs += 4;
			verts[offs].uv_0.x = -verts[offs].position.y;
			verts[offs].uv_0.y = verts[offs].position.z;

			offs += 4;
			verts[offs].uv_0.x = -verts[offs].position.x;
			verts[offs].uv_0.y = verts[offs].position.z;

			offs += 4;
			verts[offs].uv_0.x = verts[offs].position.x;
			verts[offs].uv_0.y = verts[offs].position.z;
		}
	}
	if (setup.HasBinding(MESH_BINDING_TANGENTS))
	{
		verts[0].tangent = verts[1].tangent = verts[2].tangent = verts[3].tangent = { 0.0f, 1.0f, 0.0f };
		verts[4].tangent = verts[5].tangent = verts[6].tangent = verts[7].tangent = { 0.0f, -1.0f, 0.0f };
		verts[8].tangent = verts[9].tangent = verts[10].tangent = verts[11].tangent = { -1.0f, 0.0f, 0.0f };
		verts[12].tangent = verts[13].tangent = verts[14].tangent = verts[15].tangent = { 1.0f, 0.0f, 0.0f };
	}

	int baseindex = 0;
	int baseVertex = 0;
	for (short f = 0; f < 4; ++f)
	{
		inds[baseindex + 0] = baseVertex + 0;
		inds[baseindex + 1] = baseVertex + 1;
		inds[baseindex + 2] = baseVertex + 2;

		inds[baseindex + 3] = baseVertex + 2;
		inds[baseindex + 4] = baseVertex + 3;
		inds[baseindex + 5] = baseVertex + 0;
		baseindex += 6;
		baseVertex += 4;
	}
}

void bhMesh::CreateBoxSides(glm::vec3 const& _size)
{
	return CreateBoxSides(_size.x, _size.y, _size.z);
}
