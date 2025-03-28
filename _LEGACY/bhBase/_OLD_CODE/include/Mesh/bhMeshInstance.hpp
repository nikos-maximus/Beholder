#ifndef BH_MESH_INSTANCE_HPP
#define BH_MESH_INSTANCE_HPP

#include <vector>
#include "bhHash.hpp"

class bhMaterialInstance;

////////////////////////////////////////////////////////////////////////////////
class bhMeshInstance
{
    friend class bhMeshCache;
public:
    bool SetMaterial(uint32_t primitiveIndex, const bhMaterialInstance* matInst);
    bhMeshInstance* Clone();
    uint32_t GetNumPrimitives() const { return numPrimitives; }
    const bhMaterialInstance* GetMaterial(uint32_t matIdx) const;
    bhHash_t GetMeshHash() const { return meshHash; }

protected:
private:
    bhMeshInstance() = default;
    bhMeshInstance(bhHash_t _meshHash, uint32_t _numPrimitives);
    ~bhMeshInstance();

    bhHash_t meshHash{ 0 };
    uint32_t numPrimitives{ 0 };
    std::vector<const bhMaterialInstance*> materials; //Array of materials - 1 per bhMesh::Primitive
};

#endif //BH_MESH_INSTANCE_HPP
