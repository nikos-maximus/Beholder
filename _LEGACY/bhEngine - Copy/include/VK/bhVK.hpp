#ifndef BH_VULKAN_HPP
#define BH_VULKAN_HPP

#if (_WIN32 || _WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
//#elif // TODO:
#endif

#include "bhTypes.hpp"
#include "VK/bhVKFramebuffer.hpp"

struct bhImage;
class bhVKDevice;

namespace bhVK
{
	////////////////////////////////////////////////////////////////////////////////
	struct Buffer
	{
		VkBool32 IsValid() const
		{
			return (buffer != VK_NULL_HANDLE) && (memory != VK_NULL_HANDLE) ? VK_TRUE : VK_FALSE;
		}

		VkBuffer buffer{ VK_NULL_HANDLE };
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkDeviceSize size{ 0 };
	};

	////////////////////////////////////////////////////////////////////////////////
	struct Swapchain
	{
		void Clear()
		{
			frameBuffers_v.clear();
			swapChainIdx = UINT32_MAX;
			swapChain = VK_NULL_HANDLE;
		}

		VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
		uint32_t swapChainIdx{ UINT32_MAX };
		std::vector<bhVKFramebuffer> frameBuffers_v;
	};

	////////////////////////////////////////////////////////////////////////////////
	VkShaderStageFlagBits GetShaderStage(bhShaderStage shaderType);
	const char* GetPhysicalDeviceTypeString(VkPhysicalDeviceType physicalDeviceType);
	void LogPhysicalDevice(VkPhysicalDevice pDevice);
	uint32_t GetDeviceTypeRank(VkPhysicalDeviceType type);
	VkImageType MapImageTypeToVk(int dim);
	uint32_t GetQueueFamily(VkPhysicalDevice pDevice, const VkQueueFamilyProperties& requiredProperties, VkSurfaceKHR surface);
	VkBool32 TestPhysicalDeviceExtensions(VkPhysicalDevice pDevice, const std::vector<const char*>& reqExtNames_v, const char* layerName);
	VkBool32 TestPhysicalDeviceSurfaceCaps(VkPhysicalDevice pDevice, VkSurfaceKHR surface, const VkSurfaceCapabilitiesKHR* reqCaps);
	uint32_t FindPhysicalDeviceMemoryTypeIndex(VkPhysicalDevice pDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags);
	VkBool32 IsPushConstantRangeValid(VkPhysicalDevice pDevice, const VkPushConstantRange& range);
	VkBool32 ArePushConstantRangesValid(VkPhysicalDevice pDevice, std::vector<VkPushConstantRange>& ranges_v);
	VkBool32 TestImageFormat(VkPhysicalDevice pDevice, const VkImageCreateInfo& reqImageInfo);
	VkSurfaceFormatKHR DetermineSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkBool32 IsPresentModeSupported(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode);
	VkFormat PickImageFormat(const bhImage& img);
	VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);
	void EndSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);
	bool IsTextureInputValid(const VkDescriptorImageInfo& dii);
}

#endif //BH_VULKAN_HPP
