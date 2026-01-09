#ifndef BH_VK_HPP
#define BH_VK_HPP
#include <volk.h>
#include <vk_mem_alloc.h>

class bhMesh;

struct SDL_Window;

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
    VkImageView colorView { VK_NULL_HANDLE };
    Image depthStencilImage;
    VkImageView depthStencilView { VK_NULL_HANDLE };
    VkFramebuffer framebuffer { VK_NULL_HANDLE };
  };

  bool CreateInstance();
  void DestroyInstance();
  bool CreateRenderDevice(SDL_Window* wnd);
  void DestroyRenderDevice();

  void BeginFrame();
  void EndFrame();

  struct MeshMemory
  {
    VkBuffer buffer { VK_NULL_HANDLE };
    VmaAllocation allocation { VK_NULL_HANDLE };
  };

  bool CreateMeshBuffer(bhMesh* mesh);
  void DestroyMeshBuffer(bhMesh* mesh);
}

#endif //BH_VK_HPP
