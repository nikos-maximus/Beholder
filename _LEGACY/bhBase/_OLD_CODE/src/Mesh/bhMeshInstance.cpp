#include <assert.h>
#include "Mesh/bhMeshInstance.hpp"

////////////////////////////////////////////////////////////////////////////////
bhMeshInstance::bhMeshInstance(bhHash_t _meshHash, uint32_t _numPrimitives)
	: meshHash(_meshHash)
	, numPrimitives(_numPrimitives)
	, materials(_numPrimitives)
{}

bhMeshInstance::~bhMeshInstance()
{
	materials.clear();
}

bool bhMeshInstance::SetMaterial(uint32_t primitiveIndex, const bhMaterialInstance* matInst)
{
	if (primitiveIndex < numPrimitives)
	{
		materials[primitiveIndex] = matInst;
		return true;
	}
	assert(false);
	return false;
}

bhMeshInstance* bhMeshInstance::Clone()
{
	bhMeshInstance* newInstance = new bhMeshInstance(meshHash, numPrimitives);
	for (uint32_t mi = 0; mi < numPrimitives; ++mi)
	{
		//newInstance->materials[mi] = materials[mi].Clone();
	}
	return newInstance;
}

const bhMaterialInstance* bhMeshInstance::GetMaterial(uint32_t primitiveIndex) const
{
	if (primitiveIndex < numPrimitives)
	{
		return materials[primitiveIndex];
	}
	assert(false);
	return nullptr;
}
