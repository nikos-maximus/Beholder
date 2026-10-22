#ifndef BH_VK_HPP
#define BH_VK_HPP
#include <volk.h>
#include <vk_mem_alloc.h>
#include "bhCamera.hpp"

class bhMesh;

struct SDL_Window;

namespace bhVk
{
  class GraphicsPipeline;

  static constexpr uint32_t BH_NUM_FRAMES_IN_FLIGHT { 2 };
  static constexpr VkFormat g_presentImageFormat { VK_FORMAT_B8G8R8A8_SRGB };
  static constexpr VkFormat g_depthStencilFormat { VK_FORMAT_D24_UNORM_S8_UINT };

  ////////////////////////////////////////////////////////////////////////////////
  struct Buffer
  {
    VmaAllocation allocation { VK_NULL_HANDLE };
    VkBuffer buffer { VK_NULL_HANDLE };
  };

  ////////////////////////////////////////////////////////////////////////////////
  struct Texture
  {
    VmaAllocation allocation { VK_NULL_HANDLE };
    VkImage image { VK_NULL_HANDLE };
  };

  ////////////////////////////////////////////////////////////////////////////////
  struct ShaderData
  {
    bhCamera::ViewProjection viewProj;
    glm::mat4 model;
    glm::vec4 lightPos;
    VkBool32 selected { VK_TRUE };
  };

  ////////////////////////////////////////////////////////////////////////////////
  struct ShaderDataBuffer
  {
      VmaAllocation allocation { VK_NULL_HANDLE };
      VmaAllocationInfo allocationInfo;
      VkBuffer buffer { VK_NULL_HANDLE };
      VkDeviceAddress deviceAddr;
  };

  ////////////////////////////////////////////////////////////////////////////////
  struct MeshMemory
  {
    VkBuffer buffer { VK_NULL_HANDLE };
    VmaAllocation allocation { VK_NULL_HANDLE };
  };

  ////////////////////////////////////////////////////////////////////////////////
  struct PhysicalDevice
  {
    VkPhysicalDevice device { VK_NULL_HANDLE };
    uint32_t preferredQueueFamily { 0 };
  };

  ////////////////////////////////////////////////////////////////////////////////
  class RenderDevice
  {
    friend void DestroyRenderDevice(RenderDevice*& rd);

  public:
    RenderDevice() = delete;
    RenderDevice(const PhysicalDevice& _phDevice, const VkDevice _device, const VkInstance& _instance);
    bool Init(SDL_Window* wnd);
    void Destroy();
    void BeginFrame(const GraphicsPipeline& gpl, const bhCamera* cam);
    void EndFrame();

    bool CreateMeshBuffer(bhMesh* mesh);
    void DestroyMeshBuffer(bhMesh* mesh);

    bool CreateShaderDataBuffers();
    void DestroyShaderDataBuffers();
    // bool CreateTextureFromFile(const char* filePath);

    VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& dslCI) const;
    void DestroyDescriptorSetLayout(VkDescriptorSetLayout& dsl) const;
    
    VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo& plCI) const;
    void DestroyPipelineLayout(VkPipelineLayout& pll) const;
    
    VkPipeline CreatePipeline(const VkGraphicsPipelineCreateInfo& gpCI) const;
    void DestroyPipeline(VkPipeline& pl) const;
    
    VkShaderModule CreateShaderModule(const char* filePath) const;

  protected:
    ~RenderDevice() = default;

  private:
    // bool InitImGui(SDL_Window* window);
    // void DestroyImGui();
    // void BeginImGuiFrame();
    // void EndImGuiFrame();

    bool SetupDescriptors(const VkDescriptorSetLayout& dsl);
    

    //const VkInstance& instance;
    const PhysicalDevice phDevice;
    VkDevice device { VK_NULL_HANDLE };
    VkCommandPool cmdPool { VK_NULL_HANDLE };
    VkCommandBuffer cmdBuffers[BH_NUM_FRAMES_IN_FLIGHT] { VK_NULL_HANDLE };
    VkQueue renderQueue { VK_NULL_HANDLE };
    VmaAllocator vmAllocator { VK_NULL_HANDLE };
    uint32_t imgIdx { UINT32_MAX }, frameIdx{ 0 };

    VkSemaphore semaphoresImgAcquired[BH_NUM_FRAMES_IN_FLIGHT] { VK_NULL_HANDLE };
    VkSemaphore semaphoresRenderComplete[BH_NUM_FRAMES_IN_FLIGHT] { VK_NULL_HANDLE };
    VkFence drawFences[BH_NUM_FRAMES_IN_FLIGHT] { VK_NULL_HANDLE };

    VkSwapchainKHR swapchain { VK_NULL_HANDLE };
    VkImage swapchainImages[BH_NUM_FRAMES_IN_FLIGHT] { VK_NULL_HANDLE };
    VkImageView colorViews[BH_NUM_FRAMES_IN_FLIGHT] { VK_NULL_HANDLE };
    Texture depthStencilImage;
    VkImageView depthStencilView { VK_NULL_HANDLE };

    ShaderDataBuffer shaderDataBuffers[BH_NUM_FRAMES_IN_FLIGHT];

    VkDescriptorPool descPool { VK_NULL_HANDLE };
    VkExtent2D windowSiz{};
  };

  ////////////////////////////////////////////////////////////////////////////////
  bool CreateInstance();
  void DestroyInstance();
  RenderDevice* CreateRenderDevice(SDL_Window* wnd);
  void DestroyRenderDevice(RenderDevice*& rd);
}

#endif //BH_VK_HPP
