#ifndef BH_VK_CORE_HPP
#define BH_VK_CORE_HPP

#if (_WIN32 || _WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
//#elif // TODO:
#endif

#include <volk.h>
#include <vector>
#include "bhTypes.hpp"

////////////////////////////////////////////////////////////////////////////////
struct bhTextureVK
{
	bhTextureVK() = default;

	bhTextureVK(VkImage _image, VkDeviceMemory _memory)
		: image(_image)
		, memory(_memory)
	{}

	VkImage image{ VK_NULL_HANDLE };
	VkDeviceMemory memory{ VK_NULL_HANDLE };
};

////////////////////////////////////////////////////////////////////////////////
struct bhInstanceVK
{
	VkInstance instance { VK_NULL_HANDLE };
	VkAllocationCallbacks* allocator { nullptr };
};

bhInstanceVK bhVK_InitInstance();
void bhVK_DestroyInstance(bhInstanceVK* pInstance);

////////////////////////////////////////////////////////////////////////////////
void bhVK_LogPhysicalDevice(VkPhysicalDevice physicalDevice);

struct bhSwapchainVK
{
	VkSwapchainKHR swapChain { VK_NULL_HANDLE };
	uint32_t swapChainIdx { UINT32_MAX };
	std::vector<struct bhFramebufferVK> frameBuffers_v;
};

uint32_t bhVK_GetQueueFamily(VkPhysicalDevice physicalDevice, const VkQueueFamilyProperties* requiredProperties, VkSurfaceKHR surface);
uint32_t bhVK_FindPhysicalDeviceMemoryTypeIndex(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags);
VkBool32 bhVK_TestImageFormat(VkPhysicalDevice physicalDevice, const VkImageCreateInfo& reqImageInfo);
VkBool32 bhVK_IsPresentModeSupported(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode);
VkBool32 bhVK_TestPhysicalDeviceSurfaceCaps(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const VkSurfaceCapabilitiesKHR* reqCaps);
VkBool32 bhVK_TestPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice, const std::vector<const char*>* reqExtNames_v, const char* layerName);
int bhVK_GetDeviceTypeRank(VkPhysicalDeviceType type);

VkSurfaceFormatKHR bhVK_DetermineSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);

std::vector<VkCommandBuffer> bhVK_CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t numBuffers);
void bhVK_DestroyCommandBuffers(std::vector<VkCommandBuffer>* commandBuffers_v, VkDevice device, VkCommandPool commandPool);

VkFence bhVK_CreateFence(VkDevice device, const VkAllocationCallbacks* allocator);
void bhVK_DestroyFence(VkFence fence, VkDevice device, const VkAllocationCallbacks* allocator);

VkCommandBuffer bhVK_BeginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);
void bhVK_EndSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);

void bhVK_CreateUniformBufferBinding(VkDevice device, VkDescriptorSet descSet, VkBuffer buffer, uint32_t binding);

////////////////////////////////////////////////////////////////////////////////
struct bhBufferVK
{
	VkBuffer buffer { VK_NULL_HANDLE };
	VkDeviceMemory memory { VK_NULL_HANDLE };
	VkDeviceSize size { 0 };
};

////////////////////////////////////////////////////////////////////////////////
struct bhFramebufferVK
{
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	std::vector<bhTextureVK> colorBuffers_v;
	std::vector<VkImageView> colorViews_v;
	bhTextureVK depthStencilBuffer;
	VkImageView depthStencilView = VK_NULL_HANDLE;
	VkExtent2D size = {};
	bool deleteColorBuffers = true;
};

////////////////////////////////////////////////////////////////////////////////
VkShaderStageFlagBits bhVK_GetShaderType(bhShaderType shaderType);

inline VkBool32 bhBufferVK_IsValid(const bhBufferVK* pBuffer)
{
	return (pBuffer->buffer != VK_NULL_HANDLE) && (pBuffer->memory != VK_NULL_HANDLE) ? VK_TRUE : VK_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
#ifdef BH_API_DEBUG

VkBool32 bhVK_InitDebugCallback(const bhInstanceVK* pInstance);
void bhVK_DestroyDebugCallback(const bhInstanceVK* pInstance);

#endif //BH_API_DEBUG

#endif //BH_VK_CORE_HPP
