#ifndef BH_VK_HPP
#define BH_VK_HPP
#include <volk.h>
#include <vk_mem_alloc.h>

namespace bhVk
{
  struct Buffer
  {
    VkBuffer buffer { VK_NULL_HANDLE };
    VmaAllocation allocation { VK_NULL_HANDLE };
  };

  struct Image
  {
    VkImage image { VK_NULL_HANDLE };
    VmaAllocation allocation { VK_NULL_HANDLE };
  };

  struct Framebuffer
  {
    Image depthBufferImage;
    VkImageView colorView { VK_NULL_HANDLE };
    VkImageView depthBufferView { VK_NULL_HANDLE };
  };

  bool CreateInstance();
  void DestroyInstance();
  bool CreateRenderDevice(SDL_Window* wnd);
  void DestroyRenderDevice();
}

#endif //BH_VK_HPP
