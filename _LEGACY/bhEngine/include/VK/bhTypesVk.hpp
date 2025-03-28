#ifndef BH_TYPES_VK_HPP
#define BH_TYPES_VK_HPP

#include <vector>
#include "VK/bhIncludeVk.hpp"
#include "bhTypes.hpp"

struct bhImage;
class bhVKDevice;

namespace bhVk
{
	////////////////////////////////////////////////////////////////////////////////
	struct Buffer
	{
		VkBuffer buffer{ VK_NULL_HANDLE };
		VmaAllocation alloc{ VK_NULL_HANDLE };
	};

	inline VkBool32 IsValid(const Buffer& bf)
	{
		return (bf.buffer != VK_NULL_HANDLE) && (bf.alloc != VK_NULL_HANDLE);
	}

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
	bool IsTextureInputValid(const VkDescriptorImageInfo& dii);
}

#endif //BH_TYPES_VK_HPP
