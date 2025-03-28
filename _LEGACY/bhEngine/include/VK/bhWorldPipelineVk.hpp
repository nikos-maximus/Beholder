#ifndef BH_WORLD_PIPELINE_VK_HPP
#define BH_WORLD_PIPELINE_VK_HPP

#include "VK/bhTypesVk.hpp"
#include "bhCamera.hpp"

namespace bhVk
{
  struct RenderDevice;

  struct WorldPipeline
  {
    ////////////////////////////////////////////////////////////////////////////////
    struct Material
    {
      enum SamplerIdx
      {
        SAMPLER_ALBEDO,
        SAMPLER_NORMAL_SPECULAR,

        NUM_SAMPLERS
      };

      ////////////////////////////////////////////////////////////////////////////////
      struct CreateInfo
      {
        const char* textureNames[Material::NUM_SAMPLERS] = {};
      };

      static Material* Create(const RenderDevice& rd, const CreateInfo& mci);
      static Material* Load(const RenderDevice& rd, const char* fileName);
      static void Bind(const RenderDevice& rd, const Material& mat);
      static void Destroy(const RenderDevice& rd, const Material* mat);

      ////////////////////////////////////////////////////////////////////////////////
      VkDescriptorImageInfo textureInputs_v[NUM_SAMPLERS] = {};
      VkDescriptorSet texturesDescriptorSet{ VK_NULL_HANDLE };
    };

    ////////////////////////////////////////////////////////////////////////////////
    enum DescSetIdx
    {
      DESC_SET_VIEW_PROJECTION,
      DESC_SET_TEXTURE,

      NUM_DESC_SETS
    };

    ////////////////////////////////////////////////////////////////////////////////
    static WorldPipeline* Create(const RenderDevice& rd, uint32_t w, uint32_t h);
    static void Destroy(const RenderDevice& rd, WorldPipeline*& pipeline);
    static void Bind(const RenderDevice& rd, const WorldPipeline& pipeline);
    static void SetCameraView(const RenderDevice& rd, const WorldPipeline& pipeline, const glm::mat4& view, const glm::mat4& proj);
    static VkPipelineLayout GetLayout() { return s_layout; }

    ////////////////////////////////////////////////////////////////////////////////
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkDescriptorSet descSets_v[NUM_DESC_SETS] = {};
    Buffer cameraVPBuffer; // TODO: Should keep mapped

    static VkPipelineLayout s_layout;
    static VkDescriptorSetLayout s_descSetLayouts_v[NUM_DESC_SETS];
  };

} // bhVk

#endif //BH_WORLD_PIPELINE_VK_HPP
