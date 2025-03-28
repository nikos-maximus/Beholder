#ifndef BH_VULKAN_INSTANCE_HPP
#define BH_VULKAN_INSTANCE_HPP

#include "VK/bhIncludeVk.hpp"

namespace bhVk
{
	struct Instance
	{
		VkInstance instance{ VK_NULL_HANDLE };
		VkAllocationCallbacks* alloc{ nullptr };
		VkDebugReportCallbackEXT debugReportCallback{ VK_NULL_HANDLE };

		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT{ nullptr };
		PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT{ nullptr };
	};

	inline bool IsValid(const Instance& inst) { return inst.instance != VK_NULL_HANDLE; }
	inline void DestroySurface(const Instance& inst, VkSurfaceKHR surf) { vkDestroySurfaceKHR(inst.instance, surf, inst.alloc); }

	VkBool32 CreateInstance();
	const Instance& GetInstance();
	void DestroyInstance();
	void DestroySurface(const Instance& inst, VkSurfaceKHR surf);
}

#endif //BH_VULKAN_INSTANCE_HPP
