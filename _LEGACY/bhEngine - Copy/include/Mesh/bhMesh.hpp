#ifndef BH_MESH_HPP
#define BH_MESH_HPP

#include "Mesh/bhMeshTypes.hpp"

struct aiMesh;

class bhMesh
{
public:
    bhMesh();
    bhMesh(uint32_t _numVerts, uint32_t _numInds);
    ~bhMesh();

    bhMeshSetup& Setup() { return setup; }
    const bhMeshSetup& Setup() const { return setup; }

    // MeshGen
    void CreatePlane(float _sizex, float _sizey);
    void CreatePlane(glm::vec2 const& size);
    void CreateBox(float _sizex, float _sizey, float _sizez);
    void CreateBox(glm::vec3 const& size);
    void CreateBoxSides(float _sizex, float _sizey, float _sizez);
    void CreateBoxSides(glm::vec3 const& size);

    // Assimp importing
    bool ImportAssimp(aiMesh* _aMesh);

    uint32_t GetNumVerts() const { return numVerts; }
    const bhMeshVertex* GetVerts() const { return verts; }
    uint32_t GetNumInds() const { return numInds; }
    const bhMeshIdx_t* GetInds() const { return inds; }

    bool Load(const char* fileName);
    bool Save(const char* fileName) const;

    bhMeshDeviceData deviceData;

protected:
    void ApplyVertexOffset(float xOffs, float yOffs, float zOffs);
    void ApplyVertexOffset(glm::vec3 offs);
    void ScaleUV(float xScale, float yScale);
    void ScaleUV(glm::vec2 scale);
    void ScaleUV(float uScale);

    void _Allocate(uint32_t _numVerts, uint32_t _numInds);
    void _Free();

private:
    bhMeshSetup setup;

    bhMeshVertex* verts{ nullptr };
    bhMeshIdx_t* inds{ nullptr };

    uint32_t numVerts{ 0 };
    uint32_t numInds{ 0 };
};

#endif //BH_MESH_HPP
