#include "Scene/bhStaticMesh3D.hpp"
#include "VK/bhDeviceVk.hpp"
#include "VK/bhMeshVk.hpp"

bhStaticMesh3D::bhStaticMesh3D(const bhVk::Mesh& _mesh, const bhVk::WorldPipeline::Material& _material)
  : mesh(_mesh)
  , material(_material)
{}

void bhStaticMesh3D::Render() const
{
  bhVk::WorldPipeline::Material::Bind(bhVk::GetRenderDevice(), material);
  bhVk::BindDeviceMesh(bhVk::GetRenderDevice(), mesh);
  bhVk::DrawDeviceMesh(bhVk::GetRenderDevice(), bhVk::WorldPipeline::GetLayout(), mesh, GetTransform());
}
