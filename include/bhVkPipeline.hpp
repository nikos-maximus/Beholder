#ifndef BH_VK_PIPELINE_HPP
#define BH_VK_PIPELINE_HPP
#include <volk.h>
//#include <vk_mem_alloc.h>
//#include "bhCamera.hpp"
//
//class bhMesh;
//
//struct SDL_Window;

namespace bhVk
{
  class RenderDevice;

  ////////////////////////////////////////////////////////////////////////////////
  struct PipelineLayout
  {
    VkDescriptorSetLayout dsl{ VK_NULL_HANDLE };
    VkPipelineLayout pl{ VK_NULL_HANDLE };
  };

  ////////////////////////////////////////////////////////////////////////////////
  class GraphicsPipeline
  {
  public:
    GraphicsPipeline() = delete;
    GraphicsPipeline(const PipelineLayout& _pl)
      : pl(_pl)
    {}
    
    bool Create(RenderDevice* rd);
    void Destroy(RenderDevice* rd);
    void Bind(const VkCommandBuffer& cb) const;
    void BindDescriptorSet(const VkCommandBuffer& cb, const VkDescriptorSet& ds) const;

  protected:
  private:
    const PipelineLayout pl;
    VkPipeline pipeline{ VK_NULL_HANDLE };
  };

  PipelineLayout CreatePipelineLayout(RenderDevice* rd);
  void DestroyPipelineLayout(RenderDevice* rd, PipelineLayout& pl);
}

#endif //BH_VK_PIPELINE_HPP
