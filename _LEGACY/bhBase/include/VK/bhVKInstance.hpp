#ifndef BH_INSTANCE_HPP
#define BH_INSTANCE_HPP

#include <volk.h>

namespace bhVk
{
	class Instance
	{
	public:
		VkBool32 Init();
		void Destroy();

		VkBool32 IsValid() const
		{
			return instance != VK_NULL_HANDLE;
		}

		VkInstance GetInstance() const
		{
			return instance;
		}

		VkInstance* GetInstancePtr()
		{
			return &instance;
		}

		VkAllocationCallbacks* GetAllocator() const
		{
			return allocator;
		}

	protected:
	private:
		VkBool32 InitExtensionFunctions();
		VkBool32 InitDebugCallback();
		void DestroyDebugCallback();

		VkInstance instance{ VK_NULL_HANDLE };
		VkAllocationCallbacks* allocator{ nullptr };
		VkDebugReportCallbackEXT debugReportCallback{ VK_NULL_HANDLE };

		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT{ nullptr };
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT{ nullptr };
	};
}

#endif //BH_VK_INSTANCE_HPP
