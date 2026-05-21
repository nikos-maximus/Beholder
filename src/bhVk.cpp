#include <vector>
#include <iostream> //DEBUG
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_vulkan.h>

#include "bhDefines.hpp"
#include "bhVk.hpp"
#include "bhMesh.hpp"
#include "bhUtil.hpp"
//#include "bhPlatform.hpp"
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
	constexpr VkFormat g_presentImageFormat{ VK_FORMAT_B8G8R8A8_SRGB }, g_depthStencilFormat{ VK_FORMAT_D24_UNORM_S8_UINT };
	
	static VkInstance g_instance{ VK_NULL_HANDLE };
	VkAllocationCallbacks* g_allocator{ nullptr };

	////////////////////////////////////////////////////////////////////////////////
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

	////////////////////////////////////////////////////////////////////////////////
	struct RankedPhysicalDevice
	{
		PhysicalDevice device;
		uint32_t rank{ 0 };
	};

	////////////////////////////////////////////////////////////////////////////////
	static std::vector<Texture> g_textures;




	static constexpr uint8_t BH_IMGUI_FLAG_READY = BH_BIT(0);
	static constexpr uint8_t BH_IMGUI_FLAG_VISIBLE = BH_BIT(1);

	static uint8_t g_imguiFlags = 0;

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

	RenderDevice* CreateRenderDevice(SDL_Window* wnd)
	{
		//vkEnumeratePhysicalDeviceGroups
		uint32_t numPhysDevices = 0;
		if (!Chk(vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, nullptr)))
		{
			// Figure out error?
			return nullptr;
		}

		std::vector<VkPhysicalDevice> physDevices(numPhysDevices);
		if (!Chk(vkEnumeratePhysicalDevices(g_instance, &numPhysDevices, physDevices.data())))
		{
			// Figure out error?
			return nullptr;
		}

		std::vector<RankedPhysicalDevice> rankedPhysDevices(numPhysDevices);
		for (uint32_t physDeviceIdx = 0; physDeviceIdx < numPhysDevices; ++physDeviceIdx)
		{
			rankedPhysDevices[physDeviceIdx] = GetRankedDevice(physDevices[physDeviceIdx]);
		}

		SDL_qsort(rankedPhysDevices.data(), numPhysDevices, sizeof(RankedPhysicalDevice), CompareRankedDevice);
		PhysicalDevice& selectPD = rankedPhysDevices[0].device;

		constexpr uint32_t numQueues = 1;
		const float priorities[numQueues]{ 1.0f };

		VkDeviceQueueCreateInfo queueCI{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCI.queueFamilyIndex = selectPD.preferredQueueFamily;
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

		VkDevice newDevice{ VK_NULL_HANDLE };
		if (!Chk(vkCreateDevice(selectPD.device, &deviceCI, g_allocator, &newDevice)))
		{
			// Figure out error?
			return nullptr;
		}

		RenderDevice* newRenderDevice = new RenderDevice(newDevice, selectPD, g_instance);
		if (newRenderDevice->Init(wnd))
		{
			return newRenderDevice;
		}
		DestroyRenderDevice(newRenderDevice);
		return nullptr;
	}

	void DestroyRenderDevice(RenderDevice*& rd)
	{
		rd->Destroy();
		delete rd;
		rd = nullptr;
	}

	void DestroyInstance()
	{
		vkDestroyInstance(g_instance, g_allocator);
		//SDL_Vulkan_UnloadLibrary();
		volkFinalize();
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

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	RenderDevice::RenderDevice(VkDevice _device, const PhysicalDevice& _phDevice, const VkInstance& _instance)
		: device(_device)
		, phDevice(_phDevice)
		, instance(_instance)
	{}

	bool RenderDevice::Init(SDL_Window* wnd)
	{
		////////////////////////////////////////////////////////////////////////////////
		// Create VMA
		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		{
			allocatorCreateInfo.physicalDevice = phDevice.device;
			allocatorCreateInfo.device = device;
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
		if (!Chk(vmaCreateAllocator(&allocatorCreateInfo, &vmAllocator)))
		{
			// Figure out error?
			return false;
		}

		vkGetDeviceQueue(device, phDevice.preferredQueueFamily, 0, &renderQueue);

		////////////////////////////////////////////////////////////////////////////////
		// Create Command Pool
		VkCommandPoolCreateInfo commandPoolCI{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		{
			commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolCI.queueFamilyIndex = phDevice.preferredQueueFamily;
		}
		if (!Chk(vkCreateCommandPool(device, &commandPoolCI, g_allocator, &cmdPool)))
		{
			// Figure out error?
			return false;
		}

		// Allocate command buffers
		VkCommandBufferAllocateInfo cmdBufferAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		{
			cmdBufferAI.commandPool = cmdPool;
			cmdBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAI.commandBufferCount = BH_NUM_FRAMES_IN_FLIGHT;
		}
		if (!Chk(vkAllocateCommandBuffers(device, &cmdBufferAI, cmdBuffers)))
		{
			// Figure out error?
			return false;
		}

		////////////////////////////////////////////////////////////////////////////////
		// Create swapchain
		VkSurfaceKHR wndSurface = VK_NULL_HANDLE;
		if (!SDL_Vulkan_CreateSurface(wnd, g_instance, g_allocator, &wndSurface)) return false;

		int ww, wh;
		SDL_GetWindowSize(wnd, &ww, &wh);
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
		if (Chk(vkCreateSwapchainKHR(device, &swapchainCI, g_allocator, &swapchain)))
		{
			uint32_t numImages = BH_NUM_FRAMES_IN_FLIGHT;
			if (!Chk(vkGetSwapchainImagesKHR(device, swapchain, &numImages, swapchainImages)))
			{
				return false;
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		// Create framebuffers
		for (uint32_t displayBufIdx = 0; displayBufIdx < BH_NUM_FRAMES_IN_FLIGHT; ++displayBufIdx)
		{
			VkImageViewCreateInfo colorViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			{
				colorViewCI.image = swapchainImages[displayBufIdx];
				colorViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				colorViewCI.format = g_presentImageFormat;
				//depthStencilViewCI.components = ; // All-zeroes is identity, so this should work as is (?)
				colorViewCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			}

			vkCreateImageView(device, &colorViewCI, g_allocator, &(colorViews[displayBufIdx]));
		}

		VkImageCreateInfo depthStencilImageCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		{
			depthStencilImageCI.imageType = VK_IMAGE_TYPE_2D;
			depthStencilImageCI.format = g_depthStencilFormat;
			depthStencilImageCI.extent = { wSiz.width, wSiz.height, 1 };
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

		if (Chk(vmaCreateImage(vmAllocator, &depthStencilImageCI, &allocationCI, &(depthStencilImage.image), &(depthStencilImage.allocation), nullptr)))
		{
			VkImageViewCreateInfo depthStencilViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			{
				depthStencilViewCI.image = depthStencilImage.image;
				depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				depthStencilViewCI.format = g_depthStencilFormat;
				//depthStencilViewCI.components = ; // All-zeroes is identity, so this should work as is (?)
				depthStencilViewCI.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
			}
			Chk(vkCreateImageView(device, &depthStencilViewCI, g_allocator, &depthStencilView));
		}

		//VkImageView attachments[] = { colorViews[displayBufIdx], depthStencilView };

		//VkFramebufferCreateInfo fbCI{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		//{
		//	fbCI.renderPass = g_renderPass;
		//	fbCI.attachmentCount = 2;
		//	fbCI.pAttachments = attachments;
		//	fbCI.width = g_windowSize.width;
		//	fbCI.height = g_windowSize.height;
		//	fbCI.layers = 1;
		//}

		//if (vkCreateFramebuffer(device, &fbCI, g_allocator, &(g_framebuffers[displayBufIdx].framebuffer)) != VK_SUCCESS)
		//{
		//	return false;
		//}

		////////////////////////////////////////////////////////////////////////////////
		// Create sync objects
		VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		{
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}

		bool result = true;
		for (uint32_t i = 0; i < BH_NUM_FRAMES_IN_FLIGHT; ++i)
		{
			result &= (vkCreateSemaphore(device, &semaphoreCI, g_allocator, &(semaphoresRenderFinished[i])) == VK_SUCCESS);
			result &= (vkCreateSemaphore(device, &semaphoreCI, g_allocator, &(semaphoresImageAvailable[i])) == VK_SUCCESS);
			result &= (vkCreateFence(device, &fenceCI, g_allocator, &(drawFences[i])) == VK_SUCCESS);
			if (!result) break;
		}
		return result;
	}

	void RenderDevice::Destroy()
	{
		////////////////////////////////////////////////////////////////////////////////
		// Destroy sync objects
		for (uint32_t i = 0; i < BH_NUM_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(device, semaphoresRenderFinished[i], g_allocator);
			vkDestroySemaphore(device, semaphoresImageAvailable[i], g_allocator);
			vkDestroyFence(device, drawFences[i], g_allocator);
		}

		////////////////////////////////////////////////////////////////////////////////
		// Destroy framebuffers
		vkDestroyImageView(device, depthStencilView, g_allocator);
		vmaDestroyImage(vmAllocator, depthStencilImage.image, depthStencilImage.allocation);

		//for (uint32_t colorImgIdx = 0; colorImgIdx < BH_NUM_DISPLAY_BUFFERS; ++colorImgIdx)
		//{
		//	//Framebuffer& currFB = g_framebuffers[displayBufIdx];

		//	vkDestroyImageView(device, colorViews[colorImgIdx], g_allocator);
		//	vkDestroyFramebuffer(device, currFB.framebuffer, g_allocator);
		//}

		if (vkDeviceWaitIdle(device) == VK_SUCCESS)
		{
			vkFreeCommandBuffers(device, cmdPool, BH_NUM_FRAMES_IN_FLIGHT, cmdBuffers);
			vkDestroySwapchainKHR(device, swapchain, g_allocator);
			vkDestroyCommandPool(device, cmdPool, g_allocator);
			vmaDestroyAllocator(vmAllocator);
			vkDestroyDevice(device, g_allocator);
			device = VK_NULL_HANDLE;
		}
	}

	void RenderDevice::BeginFrame(SDL_Window* wnd, const bhCamera* cam)
	{
		SDL_assert(cam);

		Chk(vkWaitForFences(device, 1, &(drawFences[imgIdx]), VK_TRUE, UINT64_MAX)); //TODO: Check
		Chk(vkResetFences(device, 1, &(drawFences[imgIdx])));
		Chk(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphoresImageAvailable[frameIdx], VK_NULL_HANDLE, &(imgIdx)));

		ShaderData shaderData;
		shaderData.viewProj = cam->GetViewProjection();
		//shaderData.model;
		memcpy(shaderDataBuffers[imgIdx].allocationInfo.pMappedData, &shaderData, sizeof(ShaderData));

		VkCommandBuffer& currCommandBuffer = cmdBuffers[imgIdx];
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
			outputBarriers[0].image = swapchainImages[imgIdx];

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
			outputBarriers[1].image = depthStencilImage.image;

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
			colorAttachmentInfo.imageView = colorViews[imgIdx];
			colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachmentInfo.clearValue = { 1.0f, 0.0f, 1.0f, 1.0f };
		}

		VkRenderingAttachmentInfo depthStencilAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		{
			depthStencilAttachmentInfo.imageView = depthStencilView;
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
		vkCmdBindIndexBuffer(currCommandBuffer, , , VK_INDEX_TYPE_UINT16);

		vkCmdPushConstants(currCommandBuffer, g_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &(shaderDataBuffers[imgIdx].deviceAddr));

		vkCmdDrawIndexed(currCommandBuffer, , 1, 0, 0, 0);
	}

	void RenderDevice::EndFrame()
	{
		VkCommandBuffer& currCommandBuffer = cmdBuffers[imgIdx];
		vkCmdEndRendering(currCommandBuffer);

		VkImageMemoryBarrier2 barrierPresent{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		{

		}
		vkEndCommandBuffer(currCommandBuffer);

		VkPipelineStageFlags waitStages{ VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		{
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &(semaphoresImageAvailable[imgIdx]);
			submitInfo.pWaitDstStageMask = &waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &(currCommandBuffer);
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &(semaphoresRenderFinished[imgIdx]);
		}

		Chk(vkQueueSubmit(renderQueue, 1, &submitInfo, drawFences[imgIdx]));

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		{
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &(semaphoresRenderFinished[imgIdx]);
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &swapchain;
			presentInfo.pImageIndices = &imgIdx;
		}
		Chk(vkQueuePresentKHR(renderQueue, &presentInfo));
		//++frameCount;
	}

	bool RenderDevice::InitImGui(SDL_Window* window)
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
				vii.PhysicalDevice = phDevice.device;
				vii.Device = device;
				vii.QueueFamily = phDevice.preferredQueueFamily;
				vii.Queue = renderQueue;
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

	void RenderDevice::DestroyImGui()
	{
		g_imguiFlags = 0;
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void RenderDevice::BeginImGuiFrame()
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

	void RenderDevice::EndImGuiFrame()
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffers[imgIdx]);
	}

	bool RenderDevice::CreateMeshBuffer(bhMesh* mesh)
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
		if (Chk(vmaCreateBuffer(vmAllocator, &bci, &aci, &(mmem->buffer), &(mmem->allocation), nullptr)))
		{
			void* bufferPtr = nullptr;
			if (Chk(vmaMapMemory(vmAllocator, mmem->allocation, &bufferPtr)))
			{
				memcpy(bufferPtr, mesh->GetVertsData(), mesh->GetVertsSiz());
				memcpy(reinterpret_cast<char*>(bufferPtr) + mesh->GetVertsSiz(), mesh->GetIndsData(), mesh->GetIndsSiz());
				vmaUnmapMemory(vmAllocator, mmem->allocation);
				mesh->GetApiImpl() = mmem;
				return true;
			}
			vmaDestroyBuffer(vmAllocator, mmem->buffer, mmem->allocation);
		}
		return false;
	}

	void RenderDevice::DestroyMeshBuffer(bhMesh* mesh)
	{
		MeshMemory*& mmem = reinterpret_cast<MeshMemory*&>(mesh->GetApiImpl());
		vmaDestroyBuffer(vmAllocator, mmem->buffer, mmem->allocation);
		delete mmem;
		mmem = nullptr;
	}

	bool RenderDevice::CreateShaderDataBuffers()
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
			ShaderDataBuffer& sd = shaderDataBuffers[dataBufferIdx];
			if (Chk(vmaCreateBuffer(vmAllocator, &bci, &aci, &(sd.buffer), &(sd.allocation), &(sd.allocationInfo))))
			{
				VkBufferDeviceAddressInfo bdaInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
				{
					bdaInfo.buffer = sd.buffer;
				}
				sd.deviceAddr = vkGetBufferDeviceAddress(device, &bdaInfo);
			}
		}

		//DEBUG
		return false;
	}

	void RenderDevice::DestroyShaderDataBuffers()
	{
		for (uint32_t dataBufferIdx = 0; dataBufferIdx < BH_NUM_FRAMES_IN_FLIGHT; ++dataBufferIdx)
		{
			ShaderDataBuffer& sd = shaderDataBuffers[dataBufferIdx];
			vmaDestroyBuffer(vmAllocator, sd.buffer, sd.allocation);
		}
	}

	bool RenderDevice::CreateTextureFromFile(const char* filePath)
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
		if (!Chk(vmaCreateImage(vmAllocator, &imgCI, &textureACI, &(newTexture.image), &(newTexture.allocation), nullptr)))
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
		if (!Chk(vkCreateImageView(device, &imgViewCI, g_allocator, &newView)))
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
		if (Chk(vmaCreateBuffer(vmAllocator, &imgSrcCI, &imgSrcACI, &(imgSrc.buffer), &(imgSrc.allocation), &allocInfo)))
		{
			return false;
		}
		memcpy(allocInfo.pMappedData, textureKTX->pData, textureKTX->dataSize);

		VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		VkFence oneTimeFence{ VK_NULL_HANDLE };
		if (!Chk(vkCreateFence(device, &fenceCI, g_allocator, &oneTimeFence)))
		{
			return false;
		}

		VkCommandBuffer oneTimeCB{ VK_NULL_HANDLE };
		VkCommandBufferAllocateInfo oneTimeCbAI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		{
			oneTimeCbAI.commandPool = cmdPool;
			oneTimeCbAI.commandBufferCount = 1;
		}
		if (!Chk(vkAllocateCommandBuffers(device, &oneTimeCbAI, &oneTimeCB)))
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
		Chk(vkQueueSubmit(renderQueue, 1, &oneTimeSI, oneTimeFence));
		Chk(vkWaitForFences(device, 1, &oneTimeFence, VK_TRUE, UINT64_MAX));

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
		Chk(vkCreateSampler(device, &samplerCI, g_allocator, &newSampler));

		ktxTexture_Destroy(textureKTX);

		g_textureDescriptors.push_back({ newSampler, newView ,VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL });

		g_textures.push_back(newTexture);
		return true;
	}

	bool RenderDevice::SetupDescriptors()
	{
		//VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlagsCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		//{
		//	descBindingFlagsCI.bindingCount = 1;
		//	descBindingFlagsCI.pBindingFlags = &descBindFlags;
		//}

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
		if (!Chk(vkCreateDescriptorPool(device, &descPoolCI, g_allocator, &descPool)))
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
		if (!Chk(vkAllocateDescriptorSets(device, &texDescSetAlloc, &descriptorSetTex)))
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
		vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);
		return true;
	}

	VkShaderModule RenderDevice::CreateShaderModule(const char* filePath)
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
		if (!Chk(vkCreateShaderModule(device, &smCI, g_allocator, &shaderModule)))
		{
			return shaderModule;
		}
		return VK_NULL_HANDLE;
	}

	VkPipelineLayout RenderDevice::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& plCI)
	{
		VkPipelineLayout layout{ VK_NULL_HANDLE };
		if (Chk(vkCreatePipelineLayout(device, &plCI, g_allocator, &layout)))
		{
			return layout;
		}
		return VK_NULL_HANDLE;
	}

	void RenderDevice::DestroyPipelineLayout(VkPipelineLayout& pl)
	{
		vkDestroyPipelineLayout(device, pl, g_allocator);
		pl = VK_NULL_HANDLE;
	}

	VkDescriptorSetLayout RenderDevice::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& dslCI)
	{
		VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
		if (Chk(vkCreateDescriptorSetLayout(device, &dslCI, g_allocator, &layout)))
		{
			return layout;
		}
		return VK_NULL_HANDLE;
	}

	void RenderDevice::DestroyDescriptorSetLayout(VkDescriptorSetLayout& dsl)
	{
		vkDestroyDescriptorSetLayout(device, dsl, g_allocator);
		dsl = VK_NULL_HANDLE;
	}

	VkPipeline RenderDevice::CreatePipeline(const VkGraphicsPipelineCreateInfo& gpCI)
	{
		VkPipeline pipeline{ VK_NULL_HANDLE };
		if (Chk(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gpCI, g_allocator, &pipeline)))
		{
			return pipeline;
		}
		return VK_NULL_HANDLE;
	}

	void RenderDevice::DestroyPipeline(VkPipeline& pl)
	{
		vkDestroyPipeline(device, pl, g_allocator);
		pl = VK_NULL_HANDLE;
	}
}
