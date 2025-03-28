#ifndef BH_WORLD_PIPELINE_HPP
#define BH_WORLD_PIPELINE_HPP

#include "bhPipeline.hpp"
#include "VK/bhVK.hpp"
#include "bhCamera.hpp"

class bhVKRenderDevice;
class bhMesh;

class bhVKWorldPipeline : public bhWorldPipeline
{
public:
  static constexpr size_t MAX_VERTS = 1024 << 1;
  static constexpr size_t MAX_TRIS = 1024 << 1;
  static constexpr size_t MAX_INDS = 3 * MAX_TRIS;

  enum DescSetIdx
  {
    DESC_SET_VIEW_PROJECTION,
    DESC_SET_TEXTURE,

    NUM_DESC_SETS
  };

  enum SamplerIdx
  {
    SAMPLER_ALBEDO,
    SAMPLER_NORMAL_SPECULAR,

    NUM_SAMPLERS
  };

  struct Material : public bhWorldMaterial
  {
    VkDescriptorImageInfo textureInputs_v[NUM_SAMPLERS] = {};
    VkDescriptorSet texturesDescriptorSet{ VK_NULL_HANDLE };
  };

  struct MaterialCreateInfo
  {
    const char* textureNames[NUM_SAMPLERS] = {};
  };

  static bhVKWorldPipeline* Create(const bhVKRenderDevice* rd, uint32_t w, uint32_t h);
  static void Bind(const bhVKRenderDevice* rd, const bhVKWorldPipeline* pipeline);
  static void Destroy(const bhVKRenderDevice* rd, bhVKWorldPipeline*& pipeline);

  void SetWorldCameraView(const bhVKRenderDevice* rd, const bhCamera::ViewData* vd) const;

  bool UploadWorldMesh(const bhVKRenderDevice* rd, bhMesh* mesh);
  void DrawWorldMesh(const bhVKRenderDevice* rd, const bhMesh* mesh, const glm::mat4& transform) const;

  static Material* CreateMaterial(const bhVKRenderDevice* rd, const MaterialCreateInfo& mci);
  static Material* LoadMaterial(const bhVKRenderDevice* rd, const char* fileName);
  static void BindMaterial(const bhVKRenderDevice* rd, const Material* mat);
  static void DestroyMaterial(const bhVKRenderDevice* rd, const Material* mat);

protected:
private:
  bhVKWorldPipeline() = default;

  static bool CreateLayout(const bhVKRenderDevice* rd);
  static void DestroyLayout(const bhVKRenderDevice* rd);

  static VkPipelineLayout s_layout;
  static VkDescriptorSetLayout s_descSetLayouts_v[NUM_DESC_SETS];
  static uint32_t s_numInst;

  VkPipeline pipeline{ VK_NULL_HANDLE };
  VkDescriptorSet descSets_v[NUM_DESC_SETS] = {};
  bhVK::Buffer cameraVPBuffer; // TODO: Should keep mapped

  bhVK::Buffer vb;
  VkDeviceSize vbMemOffs{ 0 };
  int32_t vertexOffset{ 0 };

  bhVK::Buffer ib;
  VkDeviceSize ibMemOffs{ 0 };
  uint32_t indexOffset{ 0 };
};

#endif //BH_WORLD_PIPELINE_HPP
