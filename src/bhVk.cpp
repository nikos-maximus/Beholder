#include <vector>
#include <iostream> //DEBUG
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_vulkan.h>
#include "bhVk.hpp"

namespace bhVk
{
	static constexpr uint32_t BH_VK_API_VERSION = VK_MAKE_API_VERSION(0, 1, 1, 0);
	static constexpr uint32_t NUM_DISPLAY_BUFFERS { 2 };

	static VkInstance g_instance { VK_NULL_HANDLE };
	
	static VkDevice g_renderDevice { VK_NULL_HANDLE };
	static VmaAllocator g_renderDeviceAllocator { VK_NULL_HANDLE };

	static VkCommandPool g_commandPool { VK_NULL_HANDLE };
	static VkCommandBuffer g_commandBuffers[NUM_DISPLAY_BUFFERS] { VK_NULL_HANDLE };

	static VkSwapchainKHR g_vkSwapchain { VK_NULL_HANDLE };
	static VkImage g_swapchainImages[NUM_DISPLAY_BUFFERS] { VK_NULL_HANDLE };
	static uint32_t g_currSwapImgIndex { UINT32_MAX };

	static VkQueue g_renderQueue { VK_NULL_HANDLE };

	static VkSemaphore g_smphImageAvailable { VK_NULL_HANDLE };


	__forceinline VkAllocationCallbacks* GetAllocationCallbacks()
	{
		//static VkAllocationCallbacks g_vkAllocationCallbacks = {};
		//return &g_vkAllocationCallbacks;
		return nullptr;
	}

	struct RankedPhysicalDevice
	{
		VkPhysicalDevice physDevice { VK_NULL_HANDLE };
		uint32_t rank { 0 };
		uint32_t preferredQueueFamily { 0 };
	};

	int CompareRankedDevice(const void* a, const void* b)
	{
		RankedPhysicalDevice* rda = (RankedPhysicalDevice*)a;
		RankedPhysicalDevice* rdb = (RankedPhysicalDevice*)b;

		if (rda->rank > rdb->rank) return -1;
		else if (rda->rank < rdb->rank) return 1;
		return 0;
	}

	RankedPhysicalDevice GetRankedDevice(VkPhysicalDevice device)
	{
		uint32_t rank = 0;

		VkPhysicalDeviceProperties props; //VkPhysicalDeviceProperties2
		vkGetPhysicalDeviceProperties(device, &props); //vkGetPhysicalDeviceProperties2

		//VkPhysicalDeviceFeatures features; //VkPhysicalDeviceFeatures2
		//vkGetPhysicalDeviceFeatures(currDevice, &features); //vkGetPhysicalDeviceFeatures2

		switch (props.deviceType)
		{
			case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			{
				rank += 40;
				break;
			}
			case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			{
				rank += 30;
				break;
			}
			case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			{
				rank += 20;
				break;
			}
			case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU:
			{
				rank += 10;
				break;
			}
			case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_OTHER:
			default:
			{
				break;
			}
		}

		uint32_t numQFamilies = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &numQFamilies, nullptr); //vkGetPhysicalDeviceQueueFamilyProperties2

		std::vector<VkQueueFamilyProperties> qFamilyProps(numQFamilies); //VkQueueFamilyProperties2
		vkGetPhysicalDeviceQueueFamilyProperties(device, &numQFamilies, qFamilyProps.data());

		uint32_t preferredQueueFamily = UINT32_MAX;
		for (uint32_t qf = 0; qf < numQFamilies; ++qf)
		{
			if (qFamilyProps[qf].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (preferredQueueFamily == UINT32_MAX)
				{
					preferredQueueFamily = qf;
					continue;
				}
				if (qFamilyProps[qf].queueCount > qFamilyProps[preferredQueueFamily].queueCount)
				{
					preferredQueueFamily = qf;
				}
			}
		}

		if (preferredQueueFamily == UINT32_MAX)
		{
			rank = 0;
		}
		else
		{
			rank += qFamilyProps[preferredQueueFamily].queueCount;
		}

		return { device, rank, preferredQueueFamily };
	}

	void EnumInstanceExtensions(const char* layer)
	{
		uint32_t numExtensions = 0;
		if (vkEnumerateInstanceExtensionProperties(layer, &numExtensions, nullptr) == VK_SUCCESS)
		{
			std::vector<VkExtensionProperties> extensionProps(numExtensions);
			vkEnumerateInstanceExtensionProperties(layer, &numExtensions, extensionProps.data());
			
			std::cout << "Instance Extensions - Layer : " << (layer ? layer : "no layer") << std::endl;
			for (const auto& ep : extensionProps)
			{
				std::cout << ep.extensionName << std::endl;
			}
		}
	}

	bool CreateInstance()
	{
		if (volkInitialize() != VK_SUCCESS) return false;
		//if (!SDL_Vulkan_LoadLibrary(nullptr)) return false;

		VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "Beholder";
		appInfo.applicationVersion = 1;
		appInfo.pEngineName = "BHEngine";
		appInfo.engineVersion = 1;
		appInfo.apiVersion = BH_VK_API_VERSION;

		VkInstanceCreateInfo instanceCI = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		instanceCI.pApplicationInfo = &appInfo;

		Uint32 numSDLExtensions = 0;
		auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&numSDLExtensions);

		instanceCI.enabledExtensionCount = numSDLExtensions;
		instanceCI.ppEnabledExtensionNames = sdlExtensions;

		//instanceCI.enabledLayerCount = ;
		//instanceCI.ppEnabledLayerNames = ;

		uint32_t numExtensions = 1;
#ifdef SDL_PLATFORM_WINDOWS
		++numExtensions;
#endif

		const char* extensionNames[] = {
			"VK_KHR_surface"
#ifdef SDL_PLATFORM_WINDOWS
			,"VK_KHR_win32_surface"
#endif
		};

		instanceCI.enabledExtensionCount = numExtensions;
		instanceCI.ppEnabledExtensionNames = extensionNames;

		EnumInstanceExtensions(nullptr);

		if (vkCreateInstance(&instanceCI, GetAllocationCallbacks(), &g_instance) == VK_SUCCESS)
		{
			volkLoadInstance(g_instance);
			return true;
		}
		return false;
	}

	void DestroyInstance()
	{
		//vkDeviceWaitIdle()
		vkDestroyInstance(g_instance, GetAllocationCallbacks());
		//SDL_Vulkan_UnloadLibrary();
		volkFinalize();
	}

	void DestroyRenderDevice();

	bool CreateSwapchain(SDL_Window* wnd)
	{
		VkSurfaceKHR wndSurface = VK_NULL_HANDLE;
		if (!SDL_Vulkan_CreateSurface(wnd, g_instance, GetAllocationCallbacks(), &wndSurface)) return false;

		int ww, wh;
		SDL_GetWindowSize(wnd, &ww, &wh);
		VkExtent2D wSiz = { uint32_t(ww), uint32_t(wh) };

		VkSwapchainCreateInfoKHR swapchainCI { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapchainCI.surface = wndSurface;
		swapchainCI.minImageCount = NUM_DISPLAY_BUFFERS;
		swapchainCI.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
		swapchainCI.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchainCI.imageExtent = wSiz;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//swapchainCI.queueFamilyIndexCount = ; //Relevant if not exclusive
		//swapchainCI.pQueueFamilyIndices = ;
		swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCI.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		//swapchainCI.clipped = ;
		//swapchainCI.oldSwapchain = ;

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(g_renderDevice, &swapchainCI, GetAllocationCallbacks(), &swapchain) == VK_SUCCESS)
		{
			uint32_t numImages = NUM_DISPLAY_BUFFERS;
			return vkGetSwapchainImagesKHR(g_renderDevice, swapchain, &numImages, g_swapchainImages) == VK_SUCCESS;
		}
		return false;
	}

	bool CreateFramebuffers(SDL_Window* wnd)
	{
		int ww, wh;
		SDL_GetWindowSize(wnd, &ww, &wh);

		VkImageCreateInfo depthStencilImageCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		//depthStencilImageCI

		VmaAllocationCreateInfo allocationCI = {};
		allocationCI.usage = VMA_MEMORY_USAGE_AUTO;

		Image depthStencilImage;
		vmaCreateImage(g_renderDeviceAllocator, &depthStencilImageCI, &allocationCI, &(depthStencilImage.image), &(depthStencilImage.allocation), nullptr);

		VkImageViewCreateInfo depthStencilViewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		depthStencilViewCI.image = depthStencilImage.image;
		depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilViewCI.format = VK_FORMAT_D24_UNORM_S8_UINT;
		//depthStencilViewCI.components = ; // All-zeroes is identity, so this should work as is (?)
		depthStencilViewCI.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };

		VkImageView depthStencilView = VK_NULL_HANDLE;
		vkCreateImageView(g_renderDevice,&depthStencilViewCI , GetAllocationCallbacks(), &depthStencilView);

		VkFramebufferCreateInfo fbCI = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		fbCI.renderPass = ;
		fbCI.attachmentCount = 2;
		fbCI.pAttachments = ;
		fbCI.width = ww;
		fbCI.height = wh;
		fbCI.layers = 1;

		VkFramebuffer fb = VK_NULL_HANDLE;
		return vkCreateFramebuffer(g_renderDevice, &fbCI, GetAllocationCallbacks(), &fb) == VK_SUCCESS;
	}

	void DestroyFramebuffers()
	{
		vkDestroyFramebuffer()
	}

	bool CreateRenderDevice(SDL_Window* wnd)
	{
		//vkEnumeratePhysicalDeviceGroups
		uint32_t numPhysDevices = 0;
		if (vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, nullptr) != VK_SUCCESS) return false;

		std::vector<VkPhysicalDevice> physDevices(numPhysDevices);
		if (vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, physDevices.data()) != VK_SUCCESS)
		{
			return false;
		}

		std::vector<RankedPhysicalDevice> rankedPhysDevices(numPhysDevices);
		for (uint32_t physDeviceIdx = 0; physDeviceIdx < numPhysDevices; ++physDeviceIdx)
		{
			rankedPhysDevices[physDeviceIdx] = GetRankedDevice(physDevices[physDeviceIdx]);
		}

		SDL_qsort(rankedPhysDevices.data(), numPhysDevices, sizeof(RankedPhysicalDevice), CompareRankedDevice);
		VkPhysicalDevice preferredPhysDevice = rankedPhysDevices[0].physDevice;

		std::vector<float> priorities = { 1.0f };
		VkDeviceQueueCreateInfo queueCI = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCI.queueFamilyIndex = rankedPhysDevices[0].preferredQueueFamily;
		queueCI.queueCount = static_cast<uint32_t>(priorities.size());
		queueCI.pQueuePriorities = priorities.data();

		uint32_t numExtensions = 1;
		const char* extensionNames[] = { "VK_KHR_swapchain" };

		VkDeviceCreateInfo deviceCI = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCI.queueCreateInfoCount = 1;
		deviceCI.pQueueCreateInfos = &queueCI;
		deviceCI.enabledExtensionCount = numExtensions;
		deviceCI.ppEnabledExtensionNames = extensionNames;

		if (vkCreateDevice(preferredPhysDevice, &deviceCI, GetAllocationCallbacks(), &g_renderDevice) == VK_SUCCESS)
		{
			// Create VMA
			VmaAllocatorCreateInfo allocatorCreateInfo = {};
			allocatorCreateInfo.physicalDevice = rankedPhysDevices[0].physDevice;
			allocatorCreateInfo.device = g_renderDevice;
			allocatorCreateInfo.instance = g_instance;
			allocatorCreateInfo.vulkanApiVersion = BH_VK_API_VERSION;
			//allocatorCreateInfo.flags =
			//	VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
			//	VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT |
			//	VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;

			VmaVulkanFunctions vulkanFunctions;
			if (vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions) == VK_SUCCESS)
			{
				allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
				vmaCreateAllocator(&allocatorCreateInfo, &g_renderDeviceAllocator);
			}

			// Create Command Pool
			VkCommandPoolCreateInfo commandPoolCI = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolCI.queueFamilyIndex = rankedPhysDevices[0].preferredQueueFamily;

			if (vkCreateCommandPool(g_renderDevice, &commandPoolCI, GetAllocationCallbacks(), &g_commandPool) == VK_SUCCESS)
			{
				return CreateSwapchain(wnd);
			}
			
		}
	fail:
		DestroyRenderDevice();
		return false;
	}

	void DestroyRenderDevice()
	{
		if (vkDeviceWaitIdle(g_renderDevice) == VK_SUCCESS)
		{
			vmaDestroyAllocator(g_renderDeviceAllocator);
			vkDestroyDevice(g_renderDevice, GetAllocationCallbacks());
		}
	}

	void BeginFrame()
	{
		vkAcquireNextImageKHR(g_renderDevice, g_vkSwapchain, UINT64_MAX, g_smphImageAvailable, VK_NULL_HANDLE, &g_currSwapImgIndex);

		VkCommandBufferBeginInfo commandBufferBI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		commandBufferBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(g_commandBuffers[g_currSwapImgIndex], &commandBufferBI);
		vkCmdClearAttachments(g_commandBuffers[g_currSwapImgIndex], config_ClearAttachmentCount, config_ClearAttachments, g_numClearRects, g_clearRects_v);

		// Begin render pass
		//VkRect2D renderArea =
		//{
		//	{0, 0},
		//	swapChain.frameBuffers_v[g_currSwapImgIndex].size
		//};

		//VkRenderPassBeginInfo renderPassBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		//renderPassBI.renderPass = renderPass;
		//renderPassBI.framebuffer = swapChain.frameBuffers_v[g_currSwapImgIndex].framebuffer;
		//renderPassBI.renderArea = renderArea;
		//renderPassBI.clearValueCount = (uint32_t)clearValues_v.size();
		//renderPassBI.pClearValues = clearValues_v.data();

		//vkCmdBeginRenderPass(g_commandBuffers[g_currSwapImgIndex], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
	}

	void EndFrame()
	{
		vkCmdEndRenderPass(g_commandBuffers[g_currSwapImgIndex]);

		vkEndCommandBuffer(g_commandBuffers[g_currSwapImgIndex]);

		//VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		//VkSubmitInfo submitInfo =
		//{
		//	VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr,
		//	1,
		//	&g_smphImageAvailable,
		//	waitStages,
		//	1,
		//	&(g_commandBuffers[g_currSwapImgIndex]),
		//	1,
		//	&sph_RenderFinished
		//};
		//vkQueueSubmit(renderQueue, 1, &submitInfo, drawFence);

		//vkWaitForFences(g_renderDevice, 1, &drawFence, VK_TRUE, UINT64_MAX);
		//vkResetFences(g_renderDevice, 1, &drawFence);

		//VkPresentInfoKHR presentInfo =
		//{
		//	VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr,
		//	1,
		//	&sph_RenderFinished,
		//	1,
		//	&(g_vkSwapchain),
		//	&(g_currSwapImgIndex)
		//};
		//vkQueuePresentKHR(renderQueue, &presentInfo);
		//++frameCount;
	}
}
