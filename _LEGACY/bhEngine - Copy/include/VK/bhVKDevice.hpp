#ifndef BH_VK_DEVICE_HPP
#define BH_VK_DEVICE_HPP

#include "VK/bhVK.hpp"

class bhVKDevice
{
public:
	VkPhysicalDevice PhysicalDevice() const { return physicalDevice; }
	VkDevice Device() const { return device; }
	VkAllocationCallbacks* AllocCB() const { return allocCB; }

	bhVK::Buffer CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize) const;
	VkBool32 CopyDataToBuffer(const bhVK::Buffer& buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data) const;
	void DestroyBuffer(bhVK::Buffer& buffer) const;

protected:
	bhVKDevice() = default;

	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	VkDevice device{ VK_NULL_HANDLE };
	VkAllocationCallbacks* allocCB{ nullptr };

private:
};

#endif
