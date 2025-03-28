#ifndef BH_INSTANCE_VK_HPP
#define BH_INSTANCE_VK_HPP

#include <volk.h>

////////////////////////////////////////////////////////////////////////////////
class bhInstanceVK
{
public:
	VkBool32 Init();
	void Destroy();

#ifdef BH_API_DEBUG
	VkBool32 InitDebugCallback();
	void DestroyDebugCallback();
#endif //BH_API_DEBUG

	__forceinline VkInstance GetInstance() const
	{
		assert(instance != VK_NULL_HANDLE);
		return instance;
	}

	__forceinline VkAllocationCallbacks* GetAllocator() const
	{
		return allocator;
	}

protected:
private:
	VkInstance instance{ VK_NULL_HANDLE };
	VkAllocationCallbacks* allocator{ nullptr };
};

#endif //BH_INSTANCE_VK_HPP
