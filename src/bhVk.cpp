#include <vector>
#include <iostream> //DEBUG
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_vulkan.h>

#include "bhDefines.hpp"
#include "bhVk.hpp"
#include "bhMesh.hpp"
#include "bhUtil.hpp"
#include "bhPlatform.hpp"
#include "bhLog.hpp"

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
	static constexpr uint32_t BH_VK_API_VERSION = VK_API_VERSION_1_3;
	static constexpr uint32_t BH_NUM_FRAMES_IN_FLIGHT{ 2 }; // a.k.a. Frames In Flight
	static constexpr VkFormat g_presentImageFormat{ VK_FORMAT_B8G8R8A8_SRGB }, g_depthStencilFormat{ VK_FORMAT_D24_UNORM_S8_UINT };
	
	static inline bool Chk(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			bhLog::Message(bhLog::LOG_CATEGORY_ERROR, bhLog::LOG_PRIORITY_ERROR, "Vulkan call returned %d", result);
			SDL_assert(false);
			return false;
		}
		return true;
	}

	struct PhysicalDevice
	{
		VkPhysicalDevice device{ VK_NULL_HANDLE };
		uint32_t preferredQueueFamily{ 0 };
	};
	static PhysicalDevice g_physicalRD;

	struct RankedPhysicalDevice
	{
		PhysicalDevice device;
		uint32_t rank{ 0 };
	};

	struct Device
	{
		VkDevice device{ VK_NULL_HANDLE };
		VkCommandPool cmdPool{ VK_NULL_HANDLE };
		VkCommandBuffer cmdBuffers[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
		VkQueue renderQueue{ VK_NULL_HANDLE };
		uint32_t imgIdx{ UINT32_MAX };
		uint32_t frameIdx{ 0 };
	};
	static Device g_RD;

	static std::vector<Texture> g_textures;

	static VkInstance g_instance{ VK_NULL_HANDLE };
	
	static VmaAllocator g_renderDeviceAllocator{ VK_NULL_HANDLE };

	static ShaderDataBuffer g_shaderDataBuffers[BH_NUM_FRAMES_IN_FLIGHT];

	static VkSwapchainKHR g_swapchain{ VK_NULL_HANDLE };
	static VkImage g_swapchainImages[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static VkImageView g_colorViews[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static Texture g_depthStencilImage;
	static VkImageView g_depthStencilView{ VK_NULL_HANDLE };
	static VkExtent2D g_windowSize;

	static VkSemaphore g_semaphoresImageAvailable[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static VkSemaphore g_semaphoresRenderFinished[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };
	static VkFence g_drawFences[BH_NUM_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };

	static constexpr uint8_t BH_IMGUI_FLAG_READY = BH_BIT(0);
	static constexpr uint8_t BH_IMGUI_FLAG_VISIBLE = BH_BIT(1);

	static uint8_t g_imguiFlags = 0;

	static VkAllocationCallbacks* g_allocator{ nullptr };
	static std::vector<VkDescriptorImageInfo> g_textureDescriptors;
	static VkDescriptorSetLayout g_descSetLayout{ VK_NULL_HANDLE };

	static Pipeline g_pipeline;
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
		if (Chk(vkEnumerateInstanceExtensionProperties(layer, &numExtensions, nullptr)))
		{
			std::vector<VkExtensionProperties> extensionProps(numExtensions);
			if (Chk(vkEnumerateInstanceExtensionProperties(layer, &numExtensions, extensionProps.data())))
			{
				std::cout << "Instance Extensions - Layer : " << (layer ? layer : "no layer") << std::endl;
				for (const auto& ep : extensionProps)
				{
					std::cout << ep.extensionName << std::endl;
				}
			}
		}
	}

	bool CreateInstance()
	{
		if (volkInitialize() != VK_SUCCESS) return false;
		//if (!SDL_Vulkan_LoadLibrary(nullptr)) return false;

		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		{
			appInfo.pApplicationName = "Beholder";
			appInfo.applicationVersion = 1;
			appInfo.pEngineName = "BHEngine";
			appInfo.engineVersion = 1;
			appInfo.apiVersion = BH_VK_API_VERSION;
		}

		VkInstanceCreateInfo instanceCI{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
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
		if (Chk(vkCreateInstance(&instanceCI, g_allocator, &g_instance)))
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

		VkExtent2D wSiz{ uint32_t(ww), uint32_t(wh) };

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

		if (Chk(vkCreateSwapchainKHR(g_RD.device, &swapchainCI, g_allocator, &g_swapchain)))
		{
			uint32_t numImages = BH_NUM_FRAMES_IN_FLIGHT;
			return Chk(vkGetSwapchainImagesKHR(g_RD.device, g_swapchain, &numImages, g_swapchainImages));
		}
		return false;
	}

	bool CreateFramebuffers()
	{
		for (uint32_t displayBufIdx = 0; displayBufIdx < BH_NUM_FRAMES_IN_FLIGHT; ++displayBufIdx)
		{
			VkImageViewCreateInfo colorViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			{
				colorViewCI.image = g_swapchainImages[displayBufIdx];
				colorViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				colorViewCI.format = g_presentImageFormat;
				//depthStencilViewCI.components = ; // All-zeroes is identity, so this should work as is (?)
				colorViewCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			}

			vkCreateImageView(g_RD.device, &colorViewCI, g_allocator, &(g_colorViews[displayBufIdx]));
		}

		VkImageCreateInfo depthStencilImageCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
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

		if (Chk(vmaCreateImage(g_renderDeviceAllocator, &depthStencilImageCI, &allocationCI, &(g_depthStencilImage.image), &(g_depthStencilImage.allocation), nullptr)))
		{
			VkImageViewCreateInfo depthStencilViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			{
				depthStencilViewCI.image = g_depthStencilImage.image;
				depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				depthStencilViewCI.format = g_depthStencilFormat;
				//depthStencilViewCI.components = ; // All-zeroes is identity, so this should work as is (?)
				depthStencilViewCI.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
			}
			Chk(vkCreateImageView(g_RD.device, &depthStencilViewCI, g_allocator, &g_depthStencilView));
		}

		//VkImageView attachments[] = { g_colorViews[displayBufIdx], g_depthStencilView };

		//VkFramebufferCreateInfo fbCI{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		//{
		//	fbCI.renderPass = g_renderPass;
		//	fbCI.attachmentCount = 2;
		//	fbCI.pAttachments = attachments;
		//	fbCI.width = g_windowSize.width;
		//	fbCI.height = g_windowSize.height;
		//	fbCI.layers = 1;
		//}

		//if (vkCreateFramebuffer(g_RD.device, &fbCI, g_allocator, &(g_framebuffers[displayBufIdx].framebuffer)) != VK_SUCCESS)
		//{
		//	return false;
		//}

		return true;
	}


	bool CreateSyncObjects()
	{
		VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		{
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}

		bool result = true;
		for (uint32_t i = 0; i < BH_NUM_FRAMES_IN_FLIGHT; ++i)
		{
			result &= (vkCreateSemaphore(g_RD.device, &semaphoreCI, g_allocator, &(g_semaphoresRenderFinished[i])) == VK_SUCCESS);
			result &= (vkCreateSemaphore(g_RD.device, &semaphoreCI, g_allocator, &(g_semaphoresImageAvailable[i])) == VK_SUCCESS);
			result &= (vkCreateFence(g_RD.device, &fenceCI, g_allocator, &(g_drawFences[i])) == VK_SUCCESS);
			if (!result) break;
		}
		return result;
	}

	bool CreateRenderDevice(SDL_Window* wnd)
	{
		//vkEnumeratePhysicalDeviceGroups
		uint32_t numPhysDevices = 0;
		if (!Chk(vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, nullptr)))
		{
			// Figure out error?
			return false;
		}

		std::vector<VkPhysicalDevice> physDevices(numPhysDevices);
		if (!Chk(vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, physDevices.data())))
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
		g_physicalRD = rankedPhysDevices[0].device;

		constexpr uint32_t numQueues = 1;
		const float priorities[numQueues]{ 1.0f };

		VkDeviceQueueCreateInfo queueCI{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCI.queueFamilyIndex = g_physicalRD.preferredQueueFamily;
		queueCI.queueCount = numQueues;
		queueCI.pQueuePriorities = priorities;

		VkPhysicalDeviceVulkan11Features device11Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		{}

		VkPhysicalDeviceVulkan12Features device12Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		{
			device12Features.pNext = &device11Features;
			device12Features.descriptorIndexing = VK_TRUE;
			device12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
			device12Features.runtimeDescriptorArray = VK_TRUE;
			device12Features.bufferDeviceAddress = VK_TRUE;
		}

		VkPhysicalDeviceVulkan13Features device13Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
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
		const char* extensionNames[]{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkDeviceCreateInfo deviceCI{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		{
			deviceCI.pNext = &device13Features;
			deviceCI.queueCreateInfoCount = 1;
			deviceCI.pQueueCreateInfos = &queueCI;
			deviceCI.enabledExtensionCount = numExtensions;
			deviceCI.ppEnabledExtensionNames = extensionNames;
			deviceCI.pEnabledFeatures = &deviceFeatures;
		}

		if (!Chk(vkCreateDevice(g_physicalRD.device, &deviceCI, g_allocator, &g_RD.device)))
		{
			// Figure out error?
			return false;
		}

		// Create VMA
		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		{
			allocatorCreateInfo.physicalDevice = g_physicalRD.device;
			allocatorCreateInfo.device = g_RD.device;
			allocatorCreateInfo.instance = g_instance;
			allocatorCreateInfo.vulkanApiVersion = BH_VK_API_VERSION;
			//allocatorCreateInfo.flags =
			//	VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
			//	VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT |
			//	VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
		}

		VmaVulkanFunctions vulkanFunctions;
		if (!Chk(vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions)))
		{
			// Figure out error?
			return false;
		}

		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
		if (!Chk(vmaCreateAllocator(&allocatorCreateInfo, &g_renderDeviceAllocator)))
		{
			// Figure out error?
			return false;
		}

		vkGetDeviceQueue(g_RD.device, g_physicalRD.preferredQueueFamily, 0, &(g_RD.renderQueue));

		// Create Command Pool
		VkCommandPoolCreateInfo commandPoolCI{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		{
			commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolCI.queueFamilyIndex = g_physicalRD.preferredQueueFamily;
		}
		if (!Chk(vkCreateCommandPool(g_RD.device, &commandPoolCI, g_allocator, &(g_RD.cmdPool))))
		{
			// Figure out error?
			return false;
		}

		VkCommandBufferAllocateInfo cmdBufferAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		{
			cmdBufferAI.commandPool = g_RD.cmdPool;
			cmdBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAI.commandBufferCount = BH_NUM_FRAMES_IN_FLIGHT;
		}

		if (!Chk(vkAllocateCommandBuffers(g_RD.device, &cmdBufferAI, g_RD.cmdBuffers)))
		{
			// Figure out error?
			return false;
		}

		if (!CreateSwapchain(wnd)) return false;
		if (!CreateFramebuffers()) return false;
		if (!CreateSyncObjects()) return false;

		return true;
	}

	void DestroyFramebuffers()
	{
		vkDestroyImageView(g_RD.device, g_depthStencilView, g_allocator);
		vmaDestroyImage(g_renderDeviceAllocator, g_depthStencilImage.image, g_depthStencilImage.allocation);

		//for (uint32_t colorImgIdx = 0; colorImgIdx < BH_NUM_DISPLAY_BUFFERS; ++colorImgIdx)
		//{
		//	//Framebuffer& currFB = g_framebuffers[displayBufIdx];

		//	vkDestroyImageView(g_RD.device, g_colorViews[colorImgIdx], g_allocator);
		//	vkDestroyFramebuffer(g_RD.device, currFB.framebuffer, g_allocator);
		//}
	}

	void DestroySyncObjects()
	{
		for (uint32_t i = 0; i < BH_NUM_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(g_RD.device, g_semaphoresRenderFinished[i], g_allocator);
			vkDestroySemaphore(g_RD.device, g_semaphoresImageAvailable[i], g_allocator);
			vkDestroyFence(g_RD.device, g_drawFences[i], g_allocator);
		}
	}

	void DestroyRenderDevice()
	{
		if (vkDeviceWaitIdle(g_RD.device) == VK_SUCCESS)
		{
			DestroySyncObjects();
			DestroyFramebuffers();
			vkFreeCommandBuffers(g_RD.device, g_RD.cmdPool, BH_NUM_FRAMES_IN_FLIGHT, g_RD.cmdBuffers);
			vkDestroySwapchainKHR(g_RD.device, g_swapchain, g_allocator);
			vkDestroyCommandPool(g_RD.device, g_RD.cmdPool, g_allocator);
			vmaDestroyAllocator(g_renderDeviceAllocator);
			vkDestroyDevice(g_RD.device, g_allocator);
		}
	}

	void DestroyInstance()
	{
		vkDestroyInstance(g_instance, g_allocator);
		//SDL_Vulkan_UnloadLibrary();
		volkFinalize();
	}

	void BeginFrame(SDL_Window* wnd, const bhCamera& cam)
	{
		Chk(vkWaitForFences(g_RD.device, 1, &(g_drawFences[g_RD.imgIdx]), VK_TRUE, UINT64_MAX)); //TODO: Check
		Chk(vkResetFences(g_RD.device, 1, &(g_drawFences[g_RD.imgIdx])));
		Chk(vkAcquireNextImageKHR(g_RD.device, g_swapchain, UINT64_MAX, g_semaphoresImageAvailable[g_RD.frameIdx], VK_NULL_HANDLE, &(g_RD.imgIdx)));

		ShaderData shaderData;
		shaderData.viewProj = cam.GetViewProjection();
		//shaderData.model;
		memcpy(g_shaderDataBuffers[g_RD.imgIdx].allocationInfo.pMappedData, &shaderData, sizeof(ShaderData));

		VkCommandBuffer& currCommandBuffer = g_RD.cmdBuffers[g_RD.imgIdx];
		Chk(vkResetCommandBuffer(currCommandBuffer, 0));
		VkCommandBufferBeginInfo commandBufferBI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		{
			commandBufferBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		if (Chk(vkBeginCommandBuffer(currCommandBuffer, &commandBufferBI)))
		{
			//Check error
			return;
		}

		VkImageMemoryBarrier2 outputBarriers[2];
		{
			outputBarriers[0] = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			outputBarriers[0].srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			outputBarriers[0].srcAccessMask = 0;
			outputBarriers[0].dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			outputBarriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			outputBarriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			outputBarriers[0].newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			outputBarriers[0].image = g_swapchainImages[g_RD.imgIdx];

			outputBarriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			outputBarriers[0].subresourceRange.levelCount = 1;
			outputBarriers[0].subresourceRange.layerCount = 1;


			outputBarriers[1] = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			outputBarriers[1].srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
			outputBarriers[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			outputBarriers[1].dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
			outputBarriers[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			outputBarriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			outputBarriers[1].newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			outputBarriers[1].image = g_depthStencilImage.image;

			outputBarriers[1].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			outputBarriers[1].subresourceRange.levelCount = 1;
			outputBarriers[1].subresourceRange.layerCount = 1;
		}

		VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		{
			dependencyInfo.imageMemoryBarrierCount = 2;
			dependencyInfo.pImageMemoryBarriers = outputBarriers;
		}
		vkCmdPipelineBarrier2(currCommandBuffer, &dependencyInfo);

		VkRenderingAttachmentInfo colorAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		{
			colorAttachmentInfo.imageView = g_colorViews[g_RD.imgIdx];
			colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachmentInfo.clearValue = { 1.0f, 0.0f, 1.0f, 1.0f };
		}

		VkRenderingAttachmentInfo depthStencilAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		{
			depthStencilAttachmentInfo.imageView = g_depthStencilView;
			depthStencilAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			depthStencilAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthStencilAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachmentInfo.clearValue = { 1.0f, 0.0f };
		}

		int ww, wh;
		SDL_GetWindowSize(wnd, &ww, &wh);
		VkRenderingInfo renderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
		{
			renderInfo.renderArea.offset = {};
			renderInfo.renderArea.extent.width = ww;
			renderInfo.renderArea.extent.height = wh;
			renderInfo.layerCount = 1;
			renderInfo.colorAttachmentCount = 1;
			renderInfo.pColorAttachments = &colorAttachmentInfo;
			renderInfo.pDepthAttachment = &depthStencilAttachmentInfo;
		}
		vkCmdBeginRendering(currCommandBuffer, &renderInfo);

		VkViewport vp{};
		{
			vp.width = float(ww);
			vp.height = float(wh);
			vp.minDepth = 0.0f;
			vp.maxDepth = 1.0f;
		}
		vkCmdSetViewport(currCommandBuffer, 0, 1, &vp);
		
		VkRect2D scissor{};
		{
			scissor.extent.width = ww;
			scissor.extent.height = wh;
		}
		vkCmdSetScissor(currCommandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(currCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline.pipeline);
		VkDeviceSize vOffset{ 0 };
		vkCmdBindDescriptorSets(currCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline.layout, 0, 1, &g_descSetLayout, 0, nullptr);
		vkCmdBindVertexBuffers(currCommandBuffer, 0, 1, , &vOffset);
		vkCmdBindIndexBuffer(currCommandBuffer,,,VK_INDEX_TYPE_UINT16);

		vkCmdPushConstants(currCommandBuffer, g_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &(g_shaderDataBuffers[g_RD.imgIdx].deviceAddr));

		vkCmdDrawIndexed(currCommandBuffer, , 1, 0, 0, 0);
	}

	void EndFrame()
	{
		VkCommandBuffer& currCommandBuffer = g_RD.cmdBuffers[g_RD.imgIdx];
		vkCmdEndRendering(currCommandBuffer);
		
		VkImageMemoryBarrier2 barrierPresent{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		{

		}
		vkEndCommandBuffer(currCommandBuffer);

		VkPipelineStageFlags waitStages{ VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		{
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &(g_semaphoresImageAvailable[g_RD.imgIdx]);
			submitInfo.pWaitDstStageMask = &waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &(currCommandBuffer);
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &(g_semaphoresRenderFinished[g_RD.imgIdx]);
		}

		Chk(vkQueueSubmit(g_RD.renderQueue, 1, &submitInfo, g_drawFences[g_RD.imgIdx]));

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		{
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &(g_semaphoresRenderFinished[g_RD.imgIdx]);
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &g_swapchain;
			presentInfo.pImageIndices = &g_RD.imgIdx;
		}
		Chk(vkQueuePresentKHR(g_RD.renderQueue, &presentInfo));
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
				pipelineInfo.PipelineRenderingCreateInfo = ; //Setup if using dynamic rendering
			}

			ImGui_ImplVulkan_InitInfo vii = {};
			{
				vii.ApiVersion = BH_VK_API_VERSION;
				vii.Instance = g_instance;
				vii.PhysicalDevice = g_physicalRD.device;
				vii.Device = g_RD.device;
				vii.QueueFamily = g_physicalRD.preferredQueueFamily;
				vii.Queue = g_RD.renderQueue;
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
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), g_RD.cmdBuffers[g_RD.imgIdx]);
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
		VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
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
		if (Chk(vmaCreateBuffer(g_renderDeviceAllocator, &bci, &aci, &(mmem->buffer), &(mmem->allocation), nullptr)))
		{
			void* bufferPtr = nullptr;
			if (Chk(vmaMapMemory(g_renderDeviceAllocator, mmem->allocation, &bufferPtr)))
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
		VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
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
			if (Chk(vmaCreateBuffer(g_renderDeviceAllocator, &bci, &aci, &(sd.buffer), &(sd.allocation), &(sd.allocationInfo))))
			{
				VkBufferDeviceAddressInfo bdaInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
				{
					bdaInfo.buffer = sd.buffer;
				}
				sd.deviceAddr = vkGetBufferDeviceAddress(g_RD.device, &bdaInfo);
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
		if (!Chk(vmaCreateImage(g_renderDeviceAllocator, &imgCI, &textureACI, &(newTexture.image), &(newTexture.allocation), nullptr)))
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
		if (!Chk(vkCreateImageView(g_RD.device, &imgViewCI, g_allocator, &newView)))
		{
			return false;
		}

		Buffer imgSrc;
		VkBufferCreateInfo imgSrcCI{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
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
		if (Chk(vmaCreateBuffer(g_renderDeviceAllocator, &imgSrcCI, &imgSrcACI, &(imgSrc.buffer), &(imgSrc.allocation), &allocInfo)))
		{
			return false;
		}
		memcpy(allocInfo.pMappedData, textureKTX->pData, textureKTX->dataSize);

		VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		VkFence oneTimeFence{ VK_NULL_HANDLE };
		if (!Chk(vkCreateFence(g_RD.device, &fenceCI, g_allocator, &oneTimeFence)))
		{
			return false;
		}

		VkCommandBuffer oneTimeCB{ VK_NULL_HANDLE };
		VkCommandBufferAllocateInfo oneTimeCbAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		{
			oneTimeCbAI.commandPool = g_RD.cmdPool;
			oneTimeCbAI.commandBufferCount = 1;
		}
		if (!Chk(vkAllocateCommandBuffers(g_RD.device, &oneTimeCbAI, &oneTimeCB)))
		{
			return false;
		}

		VkCommandBufferBeginInfo oneTimeCbBI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		{
			oneTimeCbBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		if (Chk(vkBeginCommandBuffer(oneTimeCB, &oneTimeCbBI)))
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

			Chk(vkEndCommandBuffer(oneTimeCB));
		}

		VkSubmitInfo oneTimeSI{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		{
			oneTimeSI.commandBufferCount = 1;
			oneTimeSI.pCommandBuffers = &oneTimeCB;
		}
		Chk(vkQueueSubmit(g_RD.renderQueue, 1, &oneTimeSI, oneTimeFence));
		Chk(vkWaitForFences(g_RD.device, 1, &oneTimeFence, VK_TRUE, UINT64_MAX));

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
		Chk(vkCreateSampler(g_RD.device, &samplerCI, g_allocator, &newSampler));

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
		if (!Chk(vkCreateDescriptorSetLayout(g_RD.device, &descLayoutTexCI, nullptr, &g_descSetLayout)))
		{
			return false;
		}

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
		if (!Chk(vkCreateDescriptorPool(g_RD.device, &descPoolCI, g_allocator, &descPool)))
		{
			return false;
		}

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
			texDescSetAlloc.pSetLayouts = &g_descSetLayout;
		};
		VkDescriptorSet descriptorSetTex{ VK_NULL_HANDLE };
		if (!Chk(vkAllocateDescriptorSets(g_RD.device, &texDescSetAlloc, &descriptorSetTex)))
		{
			return false;
		}

		VkWriteDescriptorSet writeDescSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		{
			writeDescSet.dstSet = descriptorSetTex;
			writeDescSet.dstBinding = 0;
			writeDescSet.descriptorCount = uint32_t(g_textureDescriptors.size());
			writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescSet.pImageInfo = g_textureDescriptors.data();
		}
		vkUpdateDescriptorSets(g_RD.device, 1, &writeDescSet, 0, nullptr);
		return true;
	}

	VkShaderModule CreateShaderModule(const char* filePath)
	{
		bhUtil::FileData fData = bhUtil::ReadFile(filePath, true);
		if (!fData.IsValid()) return VK_NULL_HANDLE;

		if (fData.length % sizeof(*VkShaderModuleCreateInfo::pCode)) return VK_NULL_HANDLE; // SPIR-V requirement: VkShaderModuleCreateInfo::pCode is uint32_t*

		VkShaderModuleCreateInfo smCI{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		{
			smCI.codeSize = fData.length;
			smCI.pCode = static_cast<uint32_t*>(fData.data);
		}

		VkShaderModule shaderModule{ VK_NULL_HANDLE };
		if (!Chk(vkCreateShaderModule(g_RD.device, &smCI, g_allocator, &shaderModule)))
		{
			return shaderModule;
		}
		return VK_NULL_HANDLE;
	}

	bool CreatePipeline()
	{
		////////////////////////////////////////////////////////////////////////////////
		//Rendering
		VkPipelineRenderingCreateInfo renderingCI{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		{
			renderingCI.colorAttachmentCount = 1;
			renderingCI.pColorAttachmentFormats = &g_presentImageFormat;
			renderingCI.depthAttachmentFormat = g_depthStencilFormat;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Shader state
		VkShaderModule shader01 = CreateShaderModule(bhPlatform::CreateResourcePath(bhPlatform::ResourceType::RT_SHADER, "Shader01.out"));
		if (!shader01) return false;

		VkPipelineShaderStageCreateInfo shaderStageCI[2]{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		{
			shaderStageCI[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderStageCI[0].module = shader01;
			shaderStageCI[0].pName = "main";

			shaderStageCI[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStageCI[1].module = shader01;
			shaderStageCI[1].pName = "main";
		}

		////////////////////////////////////////////////////////////////////////////////
		//Vertex input state
		VkVertexInputBindingDescription vertexBindingDesc{};
		{
			vertexBindingDesc.binding = 0;
			vertexBindingDesc.stride = sizeof(bhMesh::Vertex);
			vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

		std::vector<VkVertexInputAttributeDescription> vertexAttribs(4);
		{
			//Position
			vertexAttribs[0].location = 0;
			vertexAttribs[0].binding = 0;
			vertexAttribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			//vertexAttribs[0].offset = ; //TODO: Check runtime value

			//Normal
			vertexAttribs[1].location = 1;
			vertexAttribs[1].binding = 0;
			vertexAttribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexAttribs[1].offset = offsetof(bhMesh::Vertex, bhMesh::Vertex::normal);

			//Tangent
			vertexAttribs[2].location = 2;
			vertexAttribs[2].binding = 0;
			vertexAttribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexAttribs[2].offset = offsetof(bhMesh::Vertex, bhMesh::Vertex::tangent);

			//UV0
			vertexAttribs[3].location = 3;
			vertexAttribs[3].binding = 0;
			vertexAttribs[3].format = VK_FORMAT_R32G32_SFLOAT;
			vertexAttribs[3].offset = offsetof(bhMesh::Vertex, bhMesh::Vertex::uv0);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputStateCI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		{
			vertexInputStateCI.vertexBindingDescriptionCount = 1;
			vertexInputStateCI.pVertexBindingDescriptions = &vertexBindingDesc;
			vertexInputStateCI.vertexAttributeDescriptionCount = uint32_t(vertexAttribs.size());
			vertexInputStateCI.pVertexAttributeDescriptions = vertexAttribs.data();
		}

		////////////////////////////////////////////////////////////////////////////////
		//Input assembly state
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		{
			inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Viewport state
		VkPipelineViewportStateCreateInfo viewportStateCI{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		{
			viewportStateCI.viewportCount = 1;
			viewportStateCI.scissorCount = 1;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationStateCI{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		{
			rasterizationStateCI.lineWidth = 1.0f;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Multisample state
		VkPipelineMultisampleStateCreateInfo multisampleStateCI{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		{
			multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		}

		////////////////////////////////////////////////////////////////////////////////
		//DepthStencil state
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		{
			depthStencilStateCI.depthTestEnable = VK_TRUE;
			depthStencilStateCI.depthWriteEnable = VK_TRUE;
			depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Colorblend state
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		{
			colorBlendAttachment.colorWriteMask = 0xF;
		}

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		{
			colorBlendStateCI.attachmentCount = 1;
			colorBlendStateCI.pAttachments = &colorBlendAttachment;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Dynamic state
		std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCI{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		{
			dynamicStateCI.dynamicStateCount = uint32_t(dynamicStates.size());
			dynamicStateCI.pDynamicStates = dynamicStates.data();
		}

		////////////////////////////////////////////////////////////////////////////////
		//Layout
		VkPushConstantRange pushConstRange{};
		{
			pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			pushConstRange.size = sizeof(VkDeviceAddress);
		}

		VkPipelineLayoutCreateInfo layoutCI{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		{
			layoutCI.setLayoutCount = 1;
			layoutCI.pSetLayouts = &g_descSetLayout;
			layoutCI.pushConstantRangeCount = 1;
			layoutCI.pPushConstantRanges = &pushConstRange;
		}

		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		if (vkCreatePipelineLayout(g_RD.device, &layoutCI, g_allocator, &pipelineLayout) != VK_SUCCESS)
		{
			return false;
		}

		////////////////////////////////////////////////////////////////////////////////
		VkGraphicsPipelineCreateInfo graphicsPipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		{
			graphicsPipelineCI.pNext = &renderingCI;
			graphicsPipelineCI.stageCount = 2;
			graphicsPipelineCI.pStages = shaderStageCI;
			graphicsPipelineCI.pVertexInputState = &vertexInputStateCI;
			graphicsPipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
			graphicsPipelineCI.pViewportState = &viewportStateCI;
			graphicsPipelineCI.pRasterizationState = &rasterizationStateCI;
			graphicsPipelineCI.pMultisampleState = &multisampleStateCI;
			graphicsPipelineCI.pDepthStencilState = &depthStencilStateCI;
			graphicsPipelineCI.pColorBlendState = &colorBlendStateCI;
			graphicsPipelineCI.pDynamicState = &dynamicStateCI;
			graphicsPipelineCI.layout = pipelineLayout;
			//gpCI.renderPass = ;
			//gpCI.subpass = ;
			//gpCI.basePipelineHandle = ;
			//gpCI.basePipelineIndex = ;
		}

		return Chk(vkCreateGraphicsPipelines(g_RD.device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, g_allocator, &g_pipeline));
	}
}
