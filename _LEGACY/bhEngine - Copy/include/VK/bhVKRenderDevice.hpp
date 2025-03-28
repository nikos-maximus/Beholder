#ifndef BH_VK_RENDER_DEVICE_HPP
#define BH_VK_RENDER_DEVICE_HPP

#include "VK/bhVKDevice.hpp"
#include "bhHash.hpp"
#include "bhPipeline.hpp"

#if BH_USE_MESH_INDEX_TYPE_UNIT16
#define BH_MESH_INDEX_TYPE VK_INDEX_TYPE_UINT16
#else
#define BH_MESH_INDEX_TYPE VK_INDEX_TYPE_UINT32
#endif

struct SDL_Window;
class bhMesh;

class bhVKRenderDevice : public bhVKDevice
{
public:
	bhVKRenderDevice();

	VkRenderPass PresentRenderPass() const { return presentRenderPass; }
	VkDescriptorPool DescriptorPool() const { return descriptorPool; }
	VkCommandBuffer CurrCmdBuffer() const { return commandBuffers_v[swapChain.swapChainIdx]; }

	VkBool32 Init(SDL_Window* wnd, const VkInstance inst, const VkAllocationCallbacks* instAlloc);
	void Destroy(const VkInstance inst, const VkAllocationCallbacks* instAllocator);
	void BeginFrame();
	void EndFrame();

	bool InitImGui(SDL_Window* wnd, VkInstance* instance) const;
	void DestroyImGui() const;
	void BeginImGuiFrame() const;
	void EndImGuiFrame() const;

	const bhVKTexture* GetTexture(bhHash_t hash) const;
	const bhVKTexture* GetTexture(const char* name) const;

	bhVKTexture CreateTexture(const VkImageCreateInfo& imageCI) const;
	bhVKTexture CreateTextureFromImage(const bhImage& img, int32_t offsetx, int32_t offsety) const;
	void DestroyTexture(bhVKTexture& texture) const;

	VkShaderModule CreateShaderFromFile(const char* fileName) const;
	
	VkDescriptorImageInfo SetupTextureDescriptor(const bhVKTexture& tx) const;

protected:
private:
	VkBool32 CreateDevice(const VkInstance inst, const VkAllocationCallbacks* instAllocator);
	VkBool32 ChoosePhysicalRenderDevice(const VkInstance inst, const VkQueueFamilyProperties& reqQFP_Graphics, const std::vector<const char*>& reqExtNames_v);
	static uint32_t RankPhysicalDeviceGraphicsFeatures(const VkPhysicalDevice physicalDevice);
	VkBool32 CreateDescriptorPool();
	VkBool32 CreatePresentRenderPass();
	VkBool32 CreateSwapchain(uint32_t numImages);
	void DestroySwapchain();
	
	VkBool32 CreateCommandBuffers(uint32_t numBuffers);

	void DestroyPipeline(VkPipeline pipeline) const;

	bhVKTexture CreateMemoryBackedImage(const VkImageCreateInfo& imageCI) const;
	bhVKTexture CreateTexture(VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels) const;
	bhVK::Buffer CreateBufferFromImage(const bhImage& img) const;
	VkImageView CreateTextureImageView(const bhVKTexture& tx) const;

	float GetAnisotropy() const;

	VkSurfaceKHR wndSurface{ VK_NULL_HANDLE };

	uint32_t gfxQueueFamilyIdx{ UINT32_MAX };
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

	bhVK::Swapchain swapChain;
	VkClearValue clearValues_v[2];
	std::vector<VkCommandBuffer> commandBuffers_v;

	VkRenderPass presentRenderPass{ VK_NULL_HANDLE };
	VkQueue renderQueue{ VK_NULL_HANDLE };

	VkFence drawFence{ VK_NULL_HANDLE };
	VkSemaphore sph_ImageAvailable{ VK_NULL_HANDLE };
	VkSemaphore sph_RenderFinished{ VK_NULL_HANDLE };

	size_t frameCount{ 0 };
};

#endif //BH_VK_RENDER_DEVICE_HPP
