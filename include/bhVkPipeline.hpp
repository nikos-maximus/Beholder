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
    VkDescriptorSetLayout descSetLayout { VK_NULL_HANDLE };
    VkPipelineLayout plLayout { VK_NULL_HANDLE };
  };
  
  PipelineLayout CreatePipelineLayout(const RenderDevice* rd);
  void DestroyPipelineLayout(const RenderDevice* rd, PipelineLayout& plLayout);

  ////////////////////////////////////////////////////////////////////////////////
  class GraphicsPipeline
  {
  public:
    bool Create(const RenderDevice* rd, const PipelineLayout& plLayout);
    void Destroy(const RenderDevice* rd);
    void Bind(const VkCommandBuffer& cb) const;
    void BindDescriptorSet(const VkCommandBuffer& cb, const VkDescriptorSet& ds) const;

  protected:
  private:
    const PipelineLayout plLayout;
    VkPipeline pipeline{ VK_NULL_HANDLE };
  };
}

#endif //BH_VK_PIPELINE_HPP
