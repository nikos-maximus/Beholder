#ifndef BH_TEXTURE_HPP
#define BH_TEXTURE_HPP

#include <vulkan/vulkan.h>

struct bhTexture
{
	bhTexture() = default;

	bhTexture(VkImage _image, VkDeviceMemory _memory, uint32_t _mipLevels, VkFormat _format)
		: image(_image)
		, memory(_memory)
		, mipLevels(_mipLevels)
		, format(_format)
	{}

	VkBool32 IsValid() const
	{
		return (image != VK_NULL_HANDLE) && (memory != VK_NULL_HANDLE) && (format != VK_FORMAT_UNDEFINED) ? VK_TRUE : VK_FALSE;
	}

	VkImage image{ VK_NULL_HANDLE };
	VkDeviceMemory memory{ VK_NULL_HANDLE };
	uint32_t mipLevels{ 0 };
	VkFormat format{ VK_FORMAT_UNDEFINED };
};

#endif //BH_TEXTURE_HPP
