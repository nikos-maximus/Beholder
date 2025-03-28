#ifndef BH_MESH_OBJECT_HPP
#define BH_MESH_OBJECT_HPP

#include "Scene/bhNode3D.hpp"
#include "VK/bhWorldPipelineVk.hpp"

namespace bhVk
{
  struct Mesh;
}

class bhStaticMesh3D : public bhNode3D
{
public:
  bhStaticMesh3D(const bhVk::Mesh& _mesh, const bhVk::WorldPipeline::Material& _material);
  void Render() const;

protected:
private:
  const bhVk::Mesh& mesh;
  const bhVk::WorldPipeline::Material& material;
};

#endif //BH_MESH_OBJECT_HPP
