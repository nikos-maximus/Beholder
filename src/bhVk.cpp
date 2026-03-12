#include <vector>
#include <iostream> //DEBUG
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_vulkan.h>

#include "bhDefines.hpp"
#include "bhVk.hpp"
#include "bhMesh.hpp"

#ifdef SDL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

namespace bhVk
{
	struct RankedPhysicalDevice
	{
		VkPhysicalDevice physDevice{ VK_NULL_HANDLE };
		uint32_t rank{ 0 };
		uint32_t preferredQueueFamily{ 0 };
	};

	static constexpr uint32_t BH_VK_API_VERSION = VK_API_VERSION_1_3;
	static constexpr uint32_t BH_NUM_FRAMES_IN_FLIGHT{ 2 }; // a.k.a. Frames In Flight
	static std::vector<Texture> g_textures;

	static constexpr VkFormat g_presentImageFormat{ VK_FORMAT_B8G8R8A8_SRGB }, g_depthStencilFormat{ VK_FORMAT_D24_UNORM_S8_UINT };

	static VkInstance g_instance{ VK_NULL_HANDLE };

	static RankedPhysicalDevice g_selectedRankedRenderDevice;
	static VkDevice g_renderDevice{ VK_NULL_HANDLE };
	static VmaAllocator g_renderDeviceAllocator{ VK_NULL_HANDLE };

	static VkCommandPool g_commandPool{ VK_NULL_HANDLE };
	static ShaderDataBuffer g_shaderDataBuffers[BH_NUM_FRAMES_IN_FLIGHT];
	static VkCommandBuffer g_commandBuffers[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static VkQueue g_renderQueue{ VK_NULL_HANDLE };

	static VkSwapchainKHR g_swapchain{ VK_NULL_HANDLE };
	static VkImage g_swapchainImages[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static VkImageView g_colorViews[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static Texture g_depthStencilImage;
	static VkImageView g_depthStencilView{ VK_NULL_HANDLE };
	static VkExtent2D g_windowSize;

	static VkRenderPass g_renderPass{ VK_NULL_HANDLE };

	static uint32_t g_currSwapImgIndex{ UINT32_MAX };

	static VkSemaphore g_semaphoresImageAvailable[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static VkSemaphore g_semaphoresRenderFinished[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static VkFence g_drawFences[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };

	static constexpr uint8_t BH_IMGUI_FLAG_READY = BH_BIT(0);
	static constexpr uint8_t BH_IMGUI_FLAG_VISIBLE = BH_BIT(1);

	static uint8_t g_imguiFlags = 0;

	static VkAllocationCallbacks* g_allocator{ nullptr };
	static std::vector<VkDescriptorImageInfo> g_textureDescriptors;

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
			if ((qFamilyProps[qf].queueFlags & VK_QUEUE_GRAPHICS_BIT) && SDL_Vulkan_GetPresentationSupport(g_instance, device, qf))
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
		{
			appInfo.pApplicationName = "Beholder";
			appInfo.applicationVersion = 1;
			appInfo.pEngineName = "BHEngine";
			appInfo.engineVersion = 1;
			appInfo.apiVersion = BH_VK_API_VERSION;
		}

		VkInstanceCreateInfo instanceCI = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		{
			instanceCI.pApplicationInfo = &appInfo;
		}

		Uint32 numInstExtensions = 0;
		auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&numInstExtensions);

		instanceCI.enabledExtensionCount = numInstExtensions;
		instanceCI.ppEnabledExtensionNames = sdlExtensions;

		//instanceCI.enabledLayerCount = ;
		//instanceCI.ppEnabledLayerNames = ;

		uint32_t numExtensions = 1;
#ifdef SDL_PLATFORM_WINDOWS
		++numExtensions;
#endif

		const char* extensionNames[] = {
			VK_KHR_SURFACE_EXTENSION_NAME
#ifdef SDL_PLATFORM_WINDOWS
			,VK_KHR_WIN32_SURFACE_EXTENSION_NAME
			//,"VK_KHR_win32_surface"
#endif
		};

		instanceCI.enabledExtensionCount = numExtensions;
		instanceCI.ppEnabledExtensionNames = extensionNames;

		EnumInstanceExtensions(nullptr);

		if (vkCreateInstance(&instanceCI, g_allocator, &g_instance) == VK_SUCCESS)
		{
			volkLoadInstance(g_instance);
			return true;
		}
		return false;
	}

	bool CreateSwapchain(SDL_Window* wnd)
	{
		VkSurfaceKHR wndSurface = VK_NULL_HANDLE;
		if (!SDL_Vulkan_CreateSurface(wnd, g_instance, g_allocator, &wndSurface)) return false;

		int ww, wh;
		SDL_GetWindowSize(wnd, &ww, &wh);
		g_windowSize.width = ww;
		g_windowSize.height = wh;

		VkExtent2D wSiz = { uint32_t(ww), uint32_t(wh) };

		VkSwapchainCreateInfoKHR swapchainCI{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		{
			swapchainCI.surface = wndSurface;
			swapchainCI.minImageCount = BH_NUM_FRAMES_IN_FLIGHT;
			swapchainCI.imageFormat = g_presentImageFormat;
			swapchainCI.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			swapchainCI.imageExtent = wSiz;
			swapchainCI.imageArrayLayers = 1;
			swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//swapchainCI.queueFamilyIndexCount = ; //Relevant if not exclusive
			//swapchainCI.pQueueFamilyIndices = ;
			swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			//swapchainCI.clipped = ;
			//swapchainCI.oldSwapchain = ;
		}

		if (vkCreateSwapchainKHR(g_renderDevice, &swapchainCI, g_allocator, &g_swapchain) == VK_SUCCESS)
		{
			uint32_t numImages = BH_NUM_FRAMES_IN_FLIGHT;
			return vkGetSwapchainImagesKHR(g_renderDevice, g_swapchain, &numImages, g_swapchainImages) == VK_SUCCESS;
		}
		return false;
	}

	bool CreateRenderPass()
	{
		VkAttachmentDescription attachmentDescs[2] = {};
		{
			attachmentDescs[0].format = VK_FORMAT_R8G8B8A8_UNORM;
			attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			attachmentDescs[1].format = g_depthStencilFormat;
			attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference attachmentRefs[2] = {};
		{
			attachmentRefs[0].attachment = 0;
			attachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachmentRefs[1].attachment = 1;
			attachmentRefs[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpassDesc = {};
		{
			subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDesc.colorAttachmentCount = 1;
			subpassDesc.pColorAttachments = &(attachmentRefs[0]);
			subpassDesc.pDepthStencilAttachment = &(attachmentRefs[1]);
		}

		VkSubpassDependency subpassDep = {};
		{
			subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			//subpassDep.srcAccessMask = 0;
			subpassDep.dstSubpass = 0; //Index
			subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDep.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderPassCI = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		{
			renderPassCI.attachmentCount = 2;
			renderPassCI.pAttachments = attachmentDescs;
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDesc;
			renderPassCI.dependencyCount = 1;
			renderPassCI.pDependencies = &subpassDep;
		}

		return (vkCreateRenderPass(g_renderDevice, &renderPassCI, g_allocator, &g_renderPass) == VK_SUCCESS);
	}

	bool CreateFramebuffers()
	{
		for (uint32_t displayBufIdx = 0; displayBufIdx < BH_NUM_FRAMES_IN_FLIGHT; ++displayBufIdx)
		{
			VkImageViewCreateInfo colorViewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			{
				colorViewCI.image = g_swapchainImages[displayBufIdx];
				colorViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				colorViewCI.format = g_presentImageFormat;
				//depthStencilViewCI.components = ; // All-zeroes is identity, so this should work as is (?)
				colorViewCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			}

			vkCreateImageView(g_renderDevice, &colorViewCI, g_allocator, &(g_colorViews[displayBufIdx]));
		}

		VkImageCreateInfo depthStencilImageCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		{
			depthStencilImageCI.imageType = VK_IMAGE_TYPE_2D;
			depthStencilImageCI.format = g_depthStencilFormat;
			depthStencilImageCI.extent = { g_windowSize.width, g_windowSize.height, 1 };
			depthStencilImageCI.mipLevels = 1;
			depthStencilImageCI.arrayLayers = 1;
			depthStencilImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
			depthStencilImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			depthStencilImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depthStencilImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//depthStencilImageCI.queueFamilyIndexCount = 0;
			//depthStencilImageCI.pQueueFamilyIndices = nullptr;
			depthStencilImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		VmaAllocationCreateInfo allocationCI = {};
		allocationCI.usage = VMA_MEMORY_USAGE_AUTO;

		if (vmaCreateImage(g_renderDeviceAllocator, &depthStencilImageCI, &allocationCI, &(g_depthStencilImage.image), &(g_depthStencilImage.allocation), nullptr) == VK_SUCCESS)
		{
			VkImageViewCreateInfo depthStencilViewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			{
				depthStencilViewCI.image = g_depthStencilImage.image;
				depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				depthStencilViewCI.format = g_depthStencilFormat;
				//depthStencilViewCI.components = ; // All-zeroes is identity, so this should work as is (?)
				depthStencilViewCI.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
			}
			vkCreateImageView(g_renderDevice, &depthStencilViewCI, g_allocator, &g_depthStencilView);
		}

		//VkImageView attachments[] = { g_colorViews[displayBufIdx], g_depthStencilView };

		//VkFramebufferCreateInfo fbCI = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		//{
		//	fbCI.renderPass = g_renderPass;
		//	fbCI.attachmentCount = 2;
		//	fbCI.pAttachments = attachments;
		//	fbCI.width = g_windowSize.width;
		//	fbCI.height = g_windowSize.height;
		//	fbCI.layers = 1;
		//}

		//if (vkCreateFramebuffer(g_renderDevice, &fbCI, g_allocator, &(g_framebuffers[displayBufIdx].framebuffer)) != VK_SUCCESS)
		//{
		//	return false;
		//}

		return true;
	}


	bool CreateSyncObjects()
	{
		VkSemaphoreCreateInfo semaphoreCI = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceCI = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		{
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}

		bool result = true;
		for (uint32_t i = 0; i < BH_NUM_FRAMES_IN_FLIGHT; ++i)
		{
			result &= (vkCreateSemaphore(g_renderDevice, &semaphoreCI, g_allocator, &(g_semaphoresRenderFinished[i])) == VK_SUCCESS);
			result &= (vkCreateSemaphore(g_renderDevice, &semaphoreCI, g_allocator, &(g_semaphoresImageAvailable[i])) == VK_SUCCESS);
			result &= (vkCreateFence(g_renderDevice, &fenceCI, g_allocator, &(g_drawFences[i])) == VK_SUCCESS);
			if (!result) break;
		}
		return result;
	}

	bool CreateRenderDevice(SDL_Window* wnd)
	{
		VkResult error = VK_SUCCESS;

		//vkEnumeratePhysicalDeviceGroups
		uint32_t numPhysDevices = 0;
		error = vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, nullptr);
		if (error)
		{
			// Figure out error?
			return false;
		}

		std::vector<VkPhysicalDevice> physDevices(numPhysDevices);
		error = vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, physDevices.data());
		if (error)
		{
			// Figure out error?
			return false;
		}

		std::vector<RankedPhysicalDevice> rankedPhysDevices(numPhysDevices);
		for (uint32_t physDeviceIdx = 0; physDeviceIdx < numPhysDevices; ++physDeviceIdx)
		{
			rankedPhysDevices[physDeviceIdx] = GetRankedDevice(physDevices[physDeviceIdx]);
		}

		SDL_qsort(rankedPhysDevices.data(), numPhysDevices, sizeof(RankedPhysicalDevice), CompareRankedDevice);
		g_selectedRankedRenderDevice = rankedPhysDevices[0];

		constexpr uint32_t numQueues = 1;
		const float priorities[numQueues] = { 1.0f };

		VkDeviceQueueCreateInfo queueCI = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCI.queueFamilyIndex = rankedPhysDevices[0].preferredQueueFamily;
		queueCI.queueCount = numQueues;
		queueCI.pQueuePriorities = priorities;

		VkPhysicalDeviceVulkan11Features device11Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		{}

		VkPhysicalDeviceVulkan12Features device12Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		{
			device12Features.pNext = &device11Features;
			device12Features.descriptorIndexing = VK_TRUE;
			device12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
			device12Features.runtimeDescriptorArray = VK_TRUE;
			device12Features.bufferDeviceAddress = VK_TRUE;
		}

		VkPhysicalDeviceVulkan13Features device13Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		{
			device13Features.pNext = &device12Features;
			device13Features.synchronization2 = VK_TRUE;
			device13Features.dynamicRendering = VK_TRUE;
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		{
			deviceFeatures.samplerAnisotropy = VK_TRUE;
		}

		constexpr uint32_t numExtensions = 1;
		const char* extensionNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkDeviceCreateInfo deviceCI = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		{
			deviceCI.pNext = &device13Features;
			deviceCI.queueCreateInfoCount = 1;
			deviceCI.pQueueCreateInfos = &queueCI;
			deviceCI.enabledExtensionCount = numExtensions;
			deviceCI.ppEnabledExtensionNames = extensionNames;
			deviceCI.pEnabledFeatures = &deviceFeatures;
		}

		error = vkCreateDevice(g_selectedRankedRenderDevice.physDevice, &deviceCI, g_allocator, &g_renderDevice);
		if (error)
		{
			// Figure out error?
			return false;
		}

		// Create VMA
		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		{
			allocatorCreateInfo.physicalDevice = rankedPhysDevices[0].physDevice;
			allocatorCreateInfo.device = g_renderDevice;
			allocatorCreateInfo.instance = g_instance;
			allocatorCreateInfo.vulkanApiVersion = BH_VK_API_VERSION;
			//allocatorCreateInfo.flags =
			//	VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
			//	VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT |
			//	VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
		}

		VmaVulkanFunctions vulkanFunctions;
		error = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
		if (error)
		{
			// Figure out error?
			return false;
		}

		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
		error = vmaCreateAllocator(&allocatorCreateInfo, &g_renderDeviceAllocator);
		if (error)
		{
			// Figure out error?
			return false;
		}

		vkGetDeviceQueue(g_renderDevice, rankedPhysDevices[0].preferredQueueFamily, 0, &g_renderQueue);

		// Create Command Pool
		VkCommandPoolCreateInfo commandPoolCI = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		{
			commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolCI.queueFamilyIndex = rankedPhysDevices[0].preferredQueueFamily;
		}
		error = vkCreateCommandPool(g_renderDevice, &commandPoolCI, g_allocator, &g_commandPool);
		if (error)
		{
			// Figure out error?
			return false;
		}

		VkCommandBufferAllocateInfo cmdBufferAI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		{
			cmdBufferAI.commandPool = g_commandPool;
			cmdBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAI.commandBufferCount = BH_NUM_FRAMES_IN_FLIGHT;
		}

		error = vkAllocateCommandBuffers(g_renderDevice, &cmdBufferAI, g_commandBuffers);
		if (error)
		{
			// Figure out error?
			return false;
		}

		if (!CreateSwapchain(wnd)) return false;
		if (!CreateRenderPass()) return false;
		if (!CreateFramebuffers()) return false;
		if (!CreateSyncObjects()) return false;

		return true;
	}

	void DestroyFramebuffers()
	{
		vkDestroyImageView(g_renderDevice, g_depthStencilView, g_allocator);
		vmaDestroyImage(g_renderDeviceAllocator, g_depthStencilImage.image, g_depthStencilImage.allocation);

		//for (uint32_t colorImgIdx = 0; colorImgIdx < BH_NUM_DISPLAY_BUFFERS; ++colorImgIdx)
		//{
		//	//Framebuffer& currFB = g_framebuffers[displayBufIdx];

		//	vkDestroyImageView(g_renderDevice, g_colorViews[colorImgIdx], g_allocator);
		//	vkDestroyFramebuffer(g_renderDevice, currFB.framebuffer, g_allocator);
		//}
	}

	void DestroySyncObjects()
	{
		for (uint32_t i = 0; i < BH_NUM_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(g_renderDevice, g_semaphoresRenderFinished[i], g_allocator);
			vkDestroySemaphore(g_renderDevice, g_semaphoresImageAvailable[i], g_allocator);
			vkDestroyFence(g_renderDevice, g_drawFences[i], g_allocator);
		}
	}

	void DestroyRenderDevice()
	{
		if (vkDeviceWaitIdle(g_renderDevice) == VK_SUCCESS)
		{
			DestroySyncObjects();

			vkDestroyRenderPass(g_renderDevice, g_renderPass, g_allocator);
			DestroyFramebuffers();
			vkFreeCommandBuffers(g_renderDevice, g_commandPool, BH_NUM_FRAMES_IN_FLIGHT, g_commandBuffers);
			vkDestroySwapchainKHR(g_renderDevice, g_swapchain, g_allocator);
			vkDestroyCommandPool(g_renderDevice, g_commandPool, g_allocator);
			vmaDestroyAllocator(g_renderDeviceAllocator);
			vkDestroyDevice(g_renderDevice, g_allocator);
		}
	}

	void DestroyInstance()
	{
		vkDestroyInstance(g_instance, g_allocator);
		//SDL_Vulkan_UnloadLibrary();
		volkFinalize();
	}

	void BeginFrame()
	{
		static constexpr float CLEAR_COLOR_RED = 1.0f;
		static constexpr float CLEAR_COLOR_GREEN = 0.0f;
		static constexpr float CLEAR_COLOR_BLUE = 1.0f;
		static constexpr float CLEAR_COLOR_ALPHA = 1.0f;

		static constexpr float CLEAR_DEPTH = 1.0f;
		static constexpr uint32_t CLEAR_STENCIL = 0;

		vkAcquireNextImageKHR(g_renderDevice, g_swapchain, UINT64_MAX, g_semaphoresImageAvailable[g_currSwapImgIndex], VK_NULL_HANDLE, &g_currSwapImgIndex);

		VkCommandBuffer& currCommandBuffer = g_commandBuffers[g_currSwapImgIndex];
		VkCommandBufferBeginInfo commandBufferBI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		{
			commandBufferBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		if (vkBeginCommandBuffer(currCommandBuffer, &commandBufferBI) != VK_SUCCESS)
		{
			//Check error
			return;
		}

		VkClearValue clearValues[2] = {};
		{
			clearValues[0].color.float32[0] = CLEAR_COLOR_RED;
			clearValues[0].color.float32[1] = CLEAR_COLOR_GREEN;
			clearValues[0].color.float32[2] = CLEAR_COLOR_BLUE;
			clearValues[0].color.float32[3] = CLEAR_COLOR_ALPHA;

			clearValues[1].depthStencil.depth = CLEAR_DEPTH;
			clearValues[1].depthStencil.stencil = CLEAR_STENCIL;
		}
		VkRenderPassBeginInfo renderPassBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		{
			renderPassBI.renderPass = g_renderPass;
			//renderPassBI.framebuffer = g_framebuffers[g_currSwapImgIndex].framebuffer;
			renderPassBI.renderArea = { {}, g_windowSize };
			renderPassBI.clearValueCount = 2;
			renderPassBI.pClearValues = clearValues;
		}
		vkCmdBeginRenderPass(currCommandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

		//VkClearAttachment clearAttachments[2] = {};
		//{
		//	clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//	//clearAttachments[0].colorAttachment = ;
		//	clearAttachments[0].clearValue.color.float32[0] = CLEAR_COLOR_RED;
		//	clearAttachments[0].clearValue.color.float32[1] = CLEAR_COLOR_GREEN;
		//	clearAttachments[0].clearValue.color.float32[2] = CLEAR_COLOR_BLUE;
		//	clearAttachments[0].clearValue.color.float32[3] = CLEAR_COLOR_ALPHA;

		//	clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		//	//clearAttachments[1].colorAttachment = ;
		//	clearAttachments[1].clearValue.depthStencil.depth = CLEAR_DEPTH;
		//	clearAttachments[1].clearValue.depthStencil.stencil = CLEAR_STENCIL;
		//}

		//VkClearRect clearRects[2] = {};
		//{
		//	clearRects[0].rect.extent = g_windowSize;
		//	//clearRects[0].baseArrayLayer = ;
		//	clearRects[0].layerCount = 1;

		//	clearRects[1].rect.extent = g_windowSize;
		//	//clearRects[1].baseArrayLayer = ;
		//	clearRects[1].layerCount = 1;
		//}
		//vkCmdClearAttachments(currCommandBuffer, 2, clearAttachments, 2, clearRects);
	}

	void EndFrame()
	{
		VkCommandBuffer& currCommandBuffer = g_commandBuffers[g_currSwapImgIndex];

		vkCmdEndRenderPass(currCommandBuffer);
		vkEndCommandBuffer(currCommandBuffer);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		{
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &(g_semaphoresImageAvailable[g_currSwapImgIndex]);
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &(currCommandBuffer);
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &(g_semaphoresRenderFinished[g_currSwapImgIndex]);
		}

		VkFence& currFence = g_drawFences[g_currSwapImgIndex];
		vkQueueSubmit(g_renderQueue, 1, &submitInfo, currFence);

		vkWaitForFences(g_renderDevice, 1, &currFence, VK_TRUE, UINT64_MAX);
		vkResetFences(g_renderDevice, 1, &currFence);

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		{
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &(g_semaphoresRenderFinished[g_currSwapImgIndex]);
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &g_swapchain;
			presentInfo.pImageIndices = &g_currSwapImgIndex;
		}
		vkQueuePresentKHR(g_renderQueue, &presentInfo);
		//++frameCount;
	}

	bool InitImGui(SDL_Window* window, void* glContext)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		if (ImGui_ImplSDL3_InitForVulkan(window))
		{
			ImGui_ImplVulkan_PipelineInfo pipelineInfo = {};
			{
				pipelineInfo.RenderPass = g_renderPass;
				//pipelineInfo.PipelineRenderingCreateInfo = ; //Setup if using dynamic rendering
				//pipelineInfo.Subpass = ;
			}

			ImGui_ImplVulkan_InitInfo vii = {};
			{
				vii.ApiVersion = BH_VK_API_VERSION;
				vii.Instance = g_instance;
				vii.PhysicalDevice = g_selectedRankedRenderDevice.physDevice;
				vii.Device = g_renderDevice;
				vii.QueueFamily = g_selectedRankedRenderDevice.preferredQueueFamily;
				vii.Queue = g_renderQueue;
				//vii.DescriptorPool = ;
				vii.DescriptorPoolSize = 1;
				vii.MinImageCount = BH_NUM_FRAMES_IN_FLIGHT;
				vii.ImageCount = BH_NUM_FRAMES_IN_FLIGHT;
				vii.PipelineInfoMain = pipelineInfo;
				//vii.UseDynamicRendering = VK_TRUE; //See also pipelineInfo

				//TODO: More optional fields to set up
			}

			if (ImGui_ImplVulkan_Init(&vii))
			{
				ImGui::StyleColorsDark();
				g_imguiFlags = BH_IMGUI_FLAG_READY;
				return true;
			}
		}
		return false;
	}

	void DestroyImGui()
	{
		g_imguiFlags = 0;
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void BeginImGuiFrame()
	{
		if (g_imguiFlags == (BH_IMGUI_FLAG_READY | BH_IMGUI_FLAG_VISIBLE))
		{
			ImGui_ImplSDL3_NewFrame();
			ImGui_ImplVulkan_NewFrame();
			ImGui::NewFrame();

			// DEBUG
			bool showdemo = true;
			ImGui::ShowDemoWindow(&showdemo);
		}
	}

	void EndImGuiFrame()
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), g_commandBuffers[g_currSwapImgIndex]);
	}

	void ShowImGui(bool show)
	{
		if (show)
		{
			g_imguiFlags |= BH_IMGUI_FLAG_VISIBLE;
		}
		else
		{
			g_imguiFlags &= ~BH_IMGUI_FLAG_VISIBLE;
		}
	}

	bool CreateMeshBuffer(bhMesh* mesh)
	{
		VkBufferCreateInfo bci = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		{
			bci.size = mesh->GetVertsSiz() + mesh->GetIndsSiz();
			bci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		VmaAllocationCreateInfo aci = {};
		{
			aci.flags =
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
				VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
				VMA_ALLOCATION_CREATE_MAPPED_BIT;
			aci.usage = VMA_MEMORY_USAGE_AUTO;
		}

		MeshMemory* mmem = new MeshMemory();
		if (vmaCreateBuffer(g_renderDeviceAllocator, &bci, &aci, &(mmem->buffer), &(mmem->allocation), nullptr) == VK_SUCCESS)
		{
			void* bufferPtr = nullptr;
			if (vmaMapMemory(g_renderDeviceAllocator, mmem->allocation, &bufferPtr) == VK_SUCCESS)
			{
				memcpy(bufferPtr, mesh->GetVertsData(), mesh->GetVertsSiz());
				memcpy(reinterpret_cast<char*>(bufferPtr) + mesh->GetVertsSiz(), mesh->GetIndsData(), mesh->GetIndsSiz());
				vmaUnmapMemory(g_renderDeviceAllocator, mmem->allocation);
				mesh->GetApiImpl() = mmem;
				return true;
			}
			vmaDestroyBuffer(g_renderDeviceAllocator, mmem->buffer, mmem->allocation);
		}
		return false;
	}

	void DestroyMeshBuffer(bhMesh* mesh)
	{
		MeshMemory*& mmem = reinterpret_cast<MeshMemory*&>(mesh->GetApiImpl());
		vmaDestroyBuffer(g_renderDeviceAllocator, mmem->buffer, mmem->allocation);
		delete mmem;
		mmem = nullptr;
	}

	bool CreateShaderDataBuffers()
	{
		VkBufferCreateInfo bci = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		{
			bci.size = sizeof(ShaderData);
			bci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		VmaAllocationCreateInfo aci = {};
		{
			aci.flags =
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
				VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
				VMA_ALLOCATION_CREATE_MAPPED_BIT;
			aci.usage = VMA_MEMORY_USAGE_AUTO;
		}

		for (uint32_t dataBufferIdx = 0; dataBufferIdx < BH_NUM_FRAMES_IN_FLIGHT; ++dataBufferIdx)
		{
			ShaderDataBuffer& sd = g_shaderDataBuffers[dataBufferIdx];
			if (vmaCreateBuffer(g_renderDeviceAllocator, &bci, &aci, &(sd.buffer), &(sd.allocation), &(sd.allocationInfo)) == VK_SUCCESS)
			{
				VkBufferDeviceAddressInfo bdaInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
				{
					bdaInfo.buffer = sd.buffer;
				}
				sd.deviceAddr = vkGetBufferDeviceAddress(g_renderDevice, &bdaInfo);
			}
		}

		//DEBUG
		return false;
	}

	void DestroyShaderDataBuffers()
	{
		for (uint32_t dataBufferIdx = 0; dataBufferIdx < BH_NUM_FRAMES_IN_FLIGHT; ++dataBufferIdx)
		{
			ShaderDataBuffer& sd = g_shaderDataBuffers[dataBufferIdx];
			vmaDestroyBuffer(g_renderDeviceAllocator, sd.buffer, sd.allocation);
		}
	}

	bool CreateTextureFromFile(const char* filePath)
	{
		//See also: VK_EXT_host_image_copy, VK_KHR_unified_image_layouts

		ktxTexture* textureKTX = nullptr;
		if (ktxTexture_CreateFromNamedFile(filePath, ktxTextureCreateFlagBits::KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &textureKTX) != KTX_SUCCESS)
		{
			return false;
		}

		VkImageCreateInfo imgCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		{
			imgCI.imageType = VK_IMAGE_TYPE_2D;
			imgCI.format = ktxTexture_GetVkFormat(textureKTX);
			imgCI.extent = { textureKTX->baseWidth, textureKTX->baseHeight, textureKTX->baseDepth }; SDL_assert(textureKTX->baseDepth == 1);
			imgCI.mipLevels = textureKTX->numLevels;
			imgCI.arrayLayers = textureKTX->numLayers; SDL_assert(textureKTX->numLayers == 1);
			imgCI.samples = VK_SAMPLE_COUNT_1_BIT;
			imgCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			imgCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imgCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		VmaAllocationCreateInfo textureACI;
		{
			textureACI.usage = VMA_MEMORY_USAGE_AUTO;
		}

		Texture newTexture;
		if (vmaCreateImage(g_renderDeviceAllocator, &imgCI, &textureACI, &(newTexture.image), &(newTexture.allocation), nullptr) != VK_SUCCESS)
		{
			return false;
		}

		VkImageViewCreateInfo imgViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		{
			imgViewCI.image = newTexture.image;
			imgViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imgViewCI.format = imgCI.format;
			{
				imgViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imgViewCI.subresourceRange.levelCount = textureKTX->numLevels;
				imgViewCI.subresourceRange.layerCount = textureKTX->numLayers; //SDL_assert(texture->numLayers == 1);
			}
		}
		VkImageView newView = VK_NULL_HANDLE;
		if (vkCreateImageView(g_renderDevice, &imgViewCI, g_allocator, &newView) != VK_SUCCESS)
		{
			return false;
		}

		Buffer imgSrc;
		VkBufferCreateInfo imgSrcCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		{
			imgSrcCI.size = textureKTX->dataSize;
			imgSrcCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		VmaAllocationCreateInfo imgSrcACI;
		{
			imgSrcACI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
			imgSrcACI.usage = VMA_MEMORY_USAGE_AUTO;
		}
		VmaAllocationInfo allocInfo;
		if (vmaCreateBuffer(g_renderDeviceAllocator, &imgSrcCI, &imgSrcACI, &(imgSrc.buffer), &(imgSrc.allocation), &allocInfo) != VK_SUCCESS)
		{
			return false;
		}
		memcpy(allocInfo.pMappedData, textureKTX->pData, textureKTX->dataSize);

		VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		VkFence oneTimeFence{ VK_NULL_HANDLE };
		if (vkCreateFence(g_renderDevice, &fenceCI, g_allocator, &oneTimeFence) != VK_SUCCESS)
		{
			return false;
		}

		VkCommandBuffer oneTimeCB{ VK_NULL_HANDLE };
		VkCommandBufferAllocateInfo oneTimeCbAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		{
			oneTimeCbAI.commandPool = g_commandPool;
			oneTimeCbAI.commandBufferCount = 1;
		}
		if (vkAllocateCommandBuffers(g_renderDevice, &oneTimeCbAI, &oneTimeCB) != VK_SUCCESS)
		{
			return false;
		}

		VkCommandBufferBeginInfo oneTimeCbBI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		{
			oneTimeCbBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		if (vkBeginCommandBuffer(oneTimeCB, &oneTimeCbBI) == VK_SUCCESS)
		{
			VkImageMemoryBarrier2 barrierTexImage{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			{
				barrierTexImage.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
				barrierTexImage.srcAccessMask = VK_ACCESS_2_NONE;
				barrierTexImage.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				barrierTexImage.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
				barrierTexImage.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrierTexImage.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrierTexImage.image = newTexture.image;
				{
					barrierTexImage.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					barrierTexImage.subresourceRange.levelCount = textureKTX->numLevels;
					barrierTexImage.subresourceRange.layerCount = textureKTX->numLayers; //SDL_assert(texture->numLayers == 1);
				}
			}
			VkDependencyInfo barrierTexInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
			{
				barrierTexInfo.imageMemoryBarrierCount = 1;
				barrierTexInfo.pImageMemoryBarriers = &barrierTexImage;
			}
			vkCmdPipelineBarrier2(oneTimeCB, &barrierTexInfo);

			std::vector<VkBufferImageCopy> copyRegions(textureKTX->numLevels);
			for (ktx_uint32_t l = 0; l < textureKTX->numLevels; ++l)
			{
				ktx_size_t mipOffset{ 0 };
				KTX_error_code ret = ktxTexture_GetImageOffset(textureKTX, l, 0, 0, &mipOffset);
				VkBufferImageCopy& cr = copyRegions[l];
				{
					cr.bufferOffset = mipOffset;

					cr.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					cr.imageSubresource.mipLevel = l;
					cr.imageSubresource.layerCount = 1;

					cr.imageExtent.width = textureKTX->baseWidth >> l;
					cr.imageExtent.height = textureKTX->baseHeight >> l;
					cr.imageExtent.depth = 1;
				}
			}
			vkCmdCopyBufferToImage(oneTimeCB, imgSrc.buffer, newTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, uint32_t(copyRegions.size()), copyRegions.data());

			VkImageMemoryBarrier2 barrierTexRead{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			{
				barrierTexRead.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
				barrierTexRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrierTexRead.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrierTexRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrierTexRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrierTexRead.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
				barrierTexRead.image = newTexture.image;

				barrierTexRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrierTexRead.subresourceRange.levelCount = textureKTX->numLevels;
				barrierTexRead.subresourceRange.layerCount = 1;
			}
			barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
			vkCmdPipelineBarrier2(oneTimeCB, &barrierTexInfo);

			vkEndCommandBuffer(oneTimeCB); // == VK_SUCCESS;
		}

		VkSubmitInfo oneTimeSI{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		{
			oneTimeSI.commandBufferCount = 1;
			oneTimeSI.pCommandBuffers = &oneTimeCB;
		}
		vkQueueSubmit(g_renderQueue, 1, &oneTimeSI, oneTimeFence); //TODO: Check
		vkWaitForFences(g_renderDevice, 1, &oneTimeFence, VK_TRUE, UINT64_MAX); //TODO: Check

		VkSamplerCreateInfo samplerCI{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		{
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCI.anisotropyEnable = VK_TRUE;
			samplerCI.maxAnisotropy = 8.0f;
			samplerCI.maxLod = float(textureKTX->numLevels);
		}
		VkSampler newSampler{ VK_NULL_HANDLE };
		vkCreateSampler(g_renderDevice, &samplerCI, g_allocator, &newSampler); //TODO: Check

		ktxTexture_Destroy(textureKTX);
		
		g_textureDescriptors.push_back({ newSampler,newView ,VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL });

		g_textures.push_back(newTexture);
		return true;
	}

	bool SetupDescriptors()
	{
		VkDescriptorBindingFlags descBindFlags{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
		VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlagsCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		{
			descBindingFlagsCI.bindingCount = 1;
			descBindingFlagsCI.pBindingFlags = &descBindFlags;
		}
		VkDescriptorSetLayoutBinding descLayoutBindingTex{};
		{
			descLayoutBindingTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descLayoutBindingTex.descriptorCount = uint32_t(g_textures.size());
			descLayoutBindingTex.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		VkDescriptorSetLayoutCreateInfo descLayoutTexCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		{
			descLayoutTexCI.pNext = &descBindFlags;
			descLayoutTexCI.bindingCount = 1;
			descLayoutTexCI.pBindings = &descLayoutBindingTex;
		}
		VkDescriptorSetLayout descSetLayout{ VK_NULL_HANDLE };
		vkCreateDescriptorSetLayout(g_renderDevice, &descLayoutTexCI, nullptr, &descSetLayout);

		VkDescriptorPoolSize descPoolSiz;
		{
			descPoolSiz.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descPoolSiz.descriptorCount = uint32_t(g_textures.size());
		}
		VkDescriptorPoolCreateInfo descPoolCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		{
			descPoolCI.maxSets = 1;
			descPoolCI.poolSizeCount = 1;
			descPoolCI.pPoolSizes = &descPoolSiz;
		}
		VkDescriptorPool descPool{ VK_NULL_HANDLE };
		vkCreateDescriptorPool(g_renderDevice, &descPoolCI, g_allocator, &descPool);

		uint32_t variableDescCount{ static_cast<uint32_t>(g_textures.size()) };
		VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
		{
			variableDescCountAI.descriptorSetCount = 1;
			variableDescCountAI.pDescriptorCounts = &variableDescCount;
		}
		VkDescriptorSetAllocateInfo texDescSetAlloc{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		{
			texDescSetAlloc.pNext = &variableDescCountAI;
			texDescSetAlloc.descriptorPool = descPool;
			texDescSetAlloc.descriptorSetCount = 1;
			texDescSetAlloc.pSetLayouts = &descSetLayout;
		};
		VkDescriptorSet descriptorSetTex{ VK_NULL_HANDLE };
		vkAllocateDescriptorSets(g_renderDevice, &texDescSetAlloc, &descriptorSetTex);

		VkWriteDescriptorSet writeDescSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		{
			writeDescSet.dstSet = descriptorSetTex;
			writeDescSet.dstBinding = 0;
			writeDescSet.descriptorCount = uint32_t(g_textureDescriptors.size());
			writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescSet.pImageInfo = g_textureDescriptors.data();
		}
		vkUpdateDescriptorSets(g_renderDevice, 1, &writeDescSet, 0, nullptr);
	}
}
