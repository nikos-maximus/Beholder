#ifndef BH_VK_HPP
#define BH_VK_HPP
#include <volk.h>
#include <vk_mem_alloc.h>
#include "bhCamera.hpp"

class bhMesh;

struct SDL_Window;

namespace bhVk
{
  struct Buffer
  {
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VkBuffer buffer { VK_NULL_HANDLE };
  };

  struct Texture
  {
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VkImage image { VK_NULL_HANDLE };
  };

  struct ShaderData
  {
    bhCamera::ViewProjection viewProj;
    glm::mat4 model;
    glm::vec4 lightPos;
    VkBool32 selected{ VK_TRUE };
  };

  struct ShaderDataBuffer
  {
      VmaAllocation allocation{ VK_NULL_HANDLE };
      VmaAllocationInfo allocationInfo;
      VkBuffer buffer{ VK_NULL_HANDLE };
      VkDeviceAddress deviceAddr;
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

  bool CreateShaderDataBuffers();
  void DestroyShaderDataBuffers();
}

#endif //BH_VK_HPP
