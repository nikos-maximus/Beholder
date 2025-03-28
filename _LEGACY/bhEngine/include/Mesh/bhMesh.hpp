#ifndef BH_MESH_HPP
#define BH_MESH_HPP

#include "Mesh/bhMeshTypes.hpp"

struct cgltf_mesh;

class bhMesh
{
public:
  struct Primitive
  {
    uint32_t baseVertex{ 0 }, numVerts{ 0 };
    uint32_t baseIndex{ 0 }, numInds{ 0 };
  };

  bhMesh() = default;
  bhMesh(uint32_t _numVerts, uint32_t _numInds, uint32_t _numPrims = 1);
  //bhMesh(bhMesh&& _mesh);
  ~bhMesh();

  bhMeshConfig& Config() { return config; }
  const bhMeshConfig& Config() const { return config; }

  uint32_t GetNumVerts() const { return numVerts; }
  const bhMeshVertex_t* GetVerts() const { return verts; }
  uint32_t GetNumInds() const { return numInds; }
  const bhMeshIdx_t* GetInds() const { return inds; }
  Primitive& GetPrimitive(uint32_t idx);
  const Primitive& GetPrimitive(uint32_t idx) const;

  bool Load(const char* fileName);
  bool Save(const char* fileName) const;

  //bhMeshDeviceData deviceData;

  static bool ImportMesh(bhMesh& mesh_bh, const cgltf_mesh& gltfMesh);

  // MeshGen
  void CreatePlane(float _sizex, float _sizey);
  void CreatePlane(glm::vec2 const& size);
  void CreateBox(float _sizex, float _sizey, float _sizez);
  void CreateBox(glm::vec3 const& size);
  void CreateBoxSides(float _sizex, float _sizey, float _sizez);
  void CreateBoxSides(glm::vec3 const& size);

protected:
  void ApplyVertexOffset(float xOffs, float yOffs, float zOffs);
  void ApplyVertexOffset(glm::vec3 offs);
  void ScaleUV(float xScale, float yScale);
  void ScaleUV(glm::vec2 scale);
  void ScaleUV(float uScale);

  void _Allocate(uint32_t _numVerts, uint32_t _numInds, uint32_t _numPrims);
  void _Free();

private:
  bhMeshConfig config;

  bhMeshVertex_t* verts{ nullptr };
  bhMeshIdx_t* inds{ nullptr };
  Primitive* prims{ nullptr };

  uint32_t numVerts{ 0 };
  uint32_t numInds{ 0 };
  uint32_t numPrims{ 0 };
};

#endif //BH_MESH_HPP
