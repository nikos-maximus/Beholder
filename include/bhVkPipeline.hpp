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

  VkPipelineLayout CreatePipelineLayout(RenderDevice* rd);
  VkPipeline CreatePipeline(RenderDevice* rd, VkPipelineLayout layout);
}

#endif //BH_VK_PIPELINE_HPP
