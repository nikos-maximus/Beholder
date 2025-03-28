#include <assert.h>
#include <glm/mat4x4.hpp>
#include "VK/bhVkRenderDevice.hpp"
#include "VK/bhVkInstance.hpp"
#include "bhConfig.h"
#include "bhUtil.hpp"
#include "bhLog.h"
#include "Texture/bhTexture.hpp"
#include "Platform/bhPlatform.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"

namespace bhVk
{
	RenderDevice::RenderDevice(Instance& _instance)
		: instance(_instance)
	{
		//DEBUG
		clearValues_v[0].color.float32[0] = 1.0f;
		clearValues_v[0].color.float32[1] = 0.0f;
		clearValues_v[0].color.float32[2] = 1.0f;
		clearValues_v[0].color.float32[3] = 1.0f;

		clearValues_v[1].depthStencil.depth = 1.0f;
		clearValues_v[1].depthStencil.stencil = 0;
	}

	VkBool32 RenderDevice::Init(VkSurfaceKHR wndSurf)
	{
		if (!instance.IsValid() || (wndSurf == VK_NULL_HANDLE))
		{
			return VK_FALSE;
		}
		wndSurface = wndSurf;
		if (!CreateDevice())
		{
			return VK_FALSE;
		}
		if (!CreatePresentRenderPass())
		{
			return VK_FALSE;
		}
		const bhRenderSettings* rs = bhConfig_GetRenderSettings();
		if (!CreateSwapchain(rs->num_swapchain_images))
		{
			return VK_FALSE;
		}

		assert(rs->num_swapchain_images == swapChain.frameBuffers_v.size());
		CreateCommandBuffers(rs->num_swapchain_images);
		vkGetDeviceQueue(device, gfxQueueFamilyIdx, 0, &renderQueue);

		// Create sync primitives
		VkFenceCreateInfo fenceCI = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		vkCreateFence(device, &fenceCI, allocator, &drawFence); // TODO: Error check

		VkSemaphoreCreateInfo semaphoreCI = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(device, &semaphoreCI, allocator, &sph_ImageAvailable); // TODO: Error check
		vkCreateSemaphore(device, &semaphoreCI, allocator, &sph_RenderFinished); // TODO: Error check

		//g_clearRects_v[0] = {};
		//g_clearRects_v[0].rect.extent.width = bhConfig::GetWidth();
		//g_clearRects_v[0].rect.extent.height = bhConfig::GetHeight();

		return VK_TRUE;
	}

	void RenderDevice::Destroy()
	{
		//textureManager.DestroyAll([this](bhTextureVK* t)
		//	{
		//		this->DestroyTexture(t);
		//	});
		DestroyWorldPipelineLayout();

		// Destroy sync primitives
		vkDestroySemaphore(device, sph_RenderFinished, allocator);
		vkDestroySemaphore(device, sph_ImageAvailable, allocator);

		vkDestroyFence(device, drawFence, allocator);

		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers_v.size()), commandBuffers_v.data());
		commandBuffers_v.clear();

		DestroySwapchain();
		vkDestroyRenderPass(device, presentRenderPass, allocator);

		vkDeviceWaitIdle(device);
		
		vkDestroyDescriptorPool(device, descriptorPool, allocator);
		vkDestroyCommandPool(device, commandPool, allocator);
		vkDestroyDevice(device, allocator);

		vkDestroySurfaceKHR(instance.GetInstance(), wndSurface, instance.GetAllocator());
		wndSurface = VK_NULL_HANDLE;
	}

	VkBool32 RenderDevice::CreateDevice()
	{
		VkQueueFamilyProperties reqQFP_Graphics = {};
		reqQFP_Graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT;
		reqQFP_Graphics.queueCount = 1;

		std::vector<const char*> reqExtNames_v = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
		if (!ChoosePhysicalRenderDevice(reqQFP_Graphics, reqExtNames_v))
		{
			return VK_FALSE;
		}

		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		//TODO - we should provide only the required features

		const bhRenderSettings* rs = bhConfig_GetRenderSettings();
		physicalDeviceFeatures.samplerAnisotropy = (rs->anisotropy_level > 0);

		std::vector<float> queuePriorities_v(reqQFP_Graphics.queueCount, 1.0f);

		VkDeviceQueueCreateInfo deviceQueueCI =
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
			gfxQueueFamilyIdx,
			reqQFP_Graphics.queueCount,
			queuePriorities_v.data()
		};

		VkDeviceCreateInfo deviceCI =
		{
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0,
			1,
			&deviceQueueCI,
			0,
			nullptr,
			(uint32_t)reqExtNames_v.size(),
			reqExtNames_v.data(),
			&physicalDeviceFeatures
		};

		if (vkCreateDevice(physicalDevice, &deviceCI, instance.GetAllocator(), &device) == VK_SUCCESS)
		{
			VkCommandPoolCreateInfo commandPoolCreateInfo =
			{
				VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				gfxQueueFamilyIdx
			};

			if (vkCreateCommandPool(device, &commandPoolCreateInfo, allocator, &commandPool) == VK_SUCCESS)
			{
				return CreateDescriptorPool();
				//return true;
			}
		}
		return VK_FALSE;
	}

	VkBool32 RenderDevice::ChoosePhysicalRenderDevice(const VkQueueFamilyProperties& reqQFP_Graphics, const std::vector<const char*>& reqExtNames_v)
	{
		uint32_t numPhysicalDevices = 0;
		if (vkEnumeratePhysicalDevices(instance.GetInstance(), &numPhysicalDevices, nullptr) != VK_SUCCESS)
		{
			// TODO: Log Error?
			return VK_FALSE;
		}
		std::vector<VkPhysicalDevice> physicalDevices_v(numPhysicalDevices);
		if (vkEnumeratePhysicalDevices(instance.GetInstance(), &numPhysicalDevices, physicalDevices_v.data()) != VK_SUCCESS)
		{
			// TODO: Log Error?
			return VK_FALSE;
		}

		std::vector<uint32_t> queueFamilyInds_v(numPhysicalDevices);
		std::vector<uint32_t> ranks_v(numPhysicalDevices);

		for (uint32_t i = 0; i < numPhysicalDevices; ++i)
		{
			ranks_v[i] = 0;
			VkPhysicalDeviceProperties pdp = {};
			vkGetPhysicalDeviceProperties(physicalDevices_v[i], &pdp);

			// TODO: Check other relevant VkPhysicalDeviceProperties
			//pdp.apiVersion;
			//pdp.driverVersion;
			//pdp.vendorID;
			//pdp.deviceID;
			//pdp.deviceType;
			//pdp.deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
			//pdp.pipelineCacheUUID[VK_UUID_SIZE];
			// --> pdp.limits;
			// --> pdp.sparseProperties;

			//VkFormatProperties pdfp = {};
			//VkFormat fmt;
			//vkGetPhysicalDeviceFormatProperties(physDevice, fmt, &pdfp);
			// TODO: Check relevant VkFormatProperties

			queueFamilyInds_v[i] = GetQueueFamily(physicalDevices_v[i], reqQFP_Graphics, wndSurface);
			if (queueFamilyInds_v[i] == UINT32_MAX)
			{
				continue;
			}
			if (TestPhysicalDeviceExtensions(physicalDevices_v[i], reqExtNames_v, nullptr) == false)
			{
				continue;
			}

			ranks_v[i] += RankPhysicalDeviceGraphicsFeatures(physicalDevices_v[i]);
			ranks_v[i] += GetDeviceTypeRank(pdp.deviceType);
		}

		uint32_t maxRankIdx = 0;
		uint32_t maxRank = ranks_v[0];
		for (uint32_t ri = 1; ri < numPhysicalDevices; ++ri)
		{
			if (ranks_v[ri] > maxRank)
			{
				maxRankIdx = ri;
			}
		}

		physicalDevice = physicalDevices_v[maxRankIdx];
		gfxQueueFamilyIdx = queueFamilyInds_v[maxRankIdx];
		if ((physicalDevice != VK_NULL_HANDLE) && (gfxQueueFamilyIdx < UINT32_MAX))
		{
			LogPhysicalDevice(physicalDevice);
			return VK_TRUE;
		}
		return VK_FALSE;
	}

	uint32_t RenderDevice::RankPhysicalDeviceGraphicsFeatures(const VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		const bhRenderSettings* rs = bhConfig_GetRenderSettings();
		uint32_t result = 0;

		VkBool32 reqAnisotropy = (rs->anisotropy_level > 0) ? VK_TRUE : VK_FALSE;
		if (physicalDeviceFeatures.samplerAnisotropy == reqAnisotropy)
		{
			++result;
		}
		return result;
	}

	VkBool32 RenderDevice::CreateDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes_v
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 }
		};
	
		uint32_t maxSets = 0;
		for (VkDescriptorPoolSize& dps : descriptorPoolSizes_v)
		{
			maxSets += dps.descriptorCount;
		}
	
		VkDescriptorPoolCreateInfo descriptorPoolCI =
		{
			// Adding VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT to flags, means that we can use vkFreeDescriptorSets
			// to return descriptor sets to the pool. This may cause fragmentation
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0,
			maxSets,
			(uint32_t)descriptorPoolSizes_v.size(),
			descriptorPoolSizes_v.data()
		};
	
		VkResult result = vkCreateDescriptorPool(device, &descriptorPoolCI, allocator, &descriptorPool);
		return (result == VK_SUCCESS) ? VK_TRUE : VK_FALSE;
	}

	VkBool32 RenderDevice::CreatePresentRenderPass()
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions_v(2);
		{
			attachmentDescriptions_v[0].flags = 0;
			attachmentDescriptions_v[0].format = DetermineSurfaceFormat(physicalDevice, wndSurface).format;
			attachmentDescriptions_v[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescriptions_v[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescriptions_v[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescriptions_v[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescriptions_v[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescriptions_v[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescriptions_v[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			attachmentDescriptions_v[1].flags = 0;
			attachmentDescriptions_v[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
			attachmentDescriptions_v[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescriptions_v[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescriptions_v[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescriptions_v[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescriptions_v[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescriptions_v[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescriptions_v[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		std::vector<VkAttachmentReference> attachmentReferences_v(2);
		{
			attachmentReferences_v[0].attachment = 0;
			attachmentReferences_v[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachmentReferences_v[1].attachment = 1;
			attachmentReferences_v[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpassDescription = {};
		{
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &(attachmentReferences_v[0]);
			subpassDescription.pDepthStencilAttachment = &(attachmentReferences_v[1]);
		}

		VkSubpassDependency subpassDependency = {};
		{
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			//subpassDependency.srcAccessMask = 0;
			subpassDependency.dstSubpass = 0; //Index
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderPassCI =
		{
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0,
			(uint32_t)attachmentDescriptions_v.size(), //attachmentCount
			attachmentDescriptions_v.data(), //pAttachments
			1, //subpassCount
			&subpassDescription, //pSubpasses
			1, //dependencyCount
			&subpassDependency //pDependencies
		};

		if (vkCreateRenderPass(device, &renderPassCI, allocator, &presentRenderPass) == VK_SUCCESS)
		{
			clearValues_v[0].color.float32[0] = 1.0f;
			clearValues_v[0].color.float32[1] = 0.0f;
			clearValues_v[0].color.float32[2] = 1.0f;
			clearValues_v[0].color.float32[3] = 1.0f;

			clearValues_v[1].depthStencil.depth = 1.0f;
			clearValues_v[1].depthStencil.stencil = 0;

			return VK_TRUE;
		}
		return VK_FALSE;
	}


	//bhVKFramebuffer RenderDevice::CreateFramebuffer(VkExtent2D size, uint32_t numColorAttachments, bool createDepthStencil)
	//{
	//	bhVKFramebuffer outFramebuffer = {};
	//	outFramebuffer.size = size;
	//	outFramebuffer.deleteColorBuffers = false;
	//
	//	VkBool32 result = VK_TRUE;
	//	result &= CreateFramebufferColorAttachments(outFramebuffer, numColorAttachments);
	//	if (createDepthStencil)
	//	{
	//		result &= CreateFramebufferDepthStencilAttachment(outFramebuffer);
	//	}
	//	assert(result == VK_TRUE);
	//	
	//	InstantiateFramebuffer(outFramebuffer);
	//	return std::move(outFramebuffer);
	//}

	VkBool32 RenderDevice::CreateFramebuffer(Framebuffer& outFramebuffer, VkExtent2D size, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil)
	{
		outFramebuffer.size = size;
		outFramebuffer.deleteColorBuffers = false;

		Texture dt;
		dt.image = image;
		outFramebuffer.colorTextures_v.push_back(dt);

		VkComponentMapping componentMapping = {};
		VkImageSubresourceRange imageSubresourceRange =
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};
		VkImageViewCreateInfo imageViewCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
			image,
			VK_IMAGE_VIEW_TYPE_2D,
			imgFormat.format,
			componentMapping,
			imageSubresourceRange
		};

		outFramebuffer.colorViews_v.resize(1);
		vkCreateImageView(device, &imageViewCI, allocator, &(outFramebuffer.colorViews_v[0]));

		if (createDepthStencil)
		{
			CreateFramebufferDepthStencilAttachment(outFramebuffer);
		}

		return InstantiateFramebuffer(outFramebuffer);
	}

	VkBool32 RenderDevice::CreateFramebufferColorAttachments(Framebuffer& outFramebuffer, uint32_t numColorAttachments)
	{
		VkImageCreateInfo imageCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_R8G8B8A8_UNORM,
			{ uint32_t(outFramebuffer.size.width), uint32_t(outFramebuffer.size.height) },
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			VK_IMAGE_LAYOUT_UNDEFINED
		};

		VkComponentMapping componentMapping = {};
		VkImageSubresourceRange imageSubresourceRange =
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};
		VkImageViewCreateInfo imageViewCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
			VK_NULL_HANDLE, //is filled in the loop below
			VK_IMAGE_VIEW_TYPE_2D,
			VK_FORMAT_R8G8B8A8_UNORM,
			componentMapping,
			imageSubresourceRange
		};

		outFramebuffer.colorTextures_v.resize(numColorAttachments);
		outFramebuffer.colorViews_v.resize(numColorAttachments);

		for (uint32_t ca = 0; ca < numColorAttachments; ++ca)
		{
			outFramebuffer.colorTextures_v[ca] = std::move(CreateTexture(imageCI));
			assert(outFramebuffer.colorTextures_v[ca].IsValid());
			imageViewCI.image = outFramebuffer.colorTextures_v[ca].image;
			if (vkCreateImageView(device, &imageViewCI, allocator, &(outFramebuffer.colorViews_v[ca])) != VK_SUCCESS)
			{
				return VK_FALSE;
			}
		}
		return VK_TRUE;
	}

	VkBool32 RenderDevice::CreateFramebufferDepthStencilAttachment(Framebuffer& outFramebuffer)
	{
		VkImageCreateInfo imageCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_D24_UNORM_S8_UINT,
			{ outFramebuffer.size.width, outFramebuffer.size.height, 1 },
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			VK_IMAGE_LAYOUT_UNDEFINED
		};

		{
			outFramebuffer.depthStencilTexture = std::move(CreateTexture(imageCI));
			assert(outFramebuffer.depthStencilTexture.IsValid());
		}

		VkComponentMapping componentMapping = {};
		VkImageSubresourceRange imageSubresourceRange =
		{
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			0,
			1,
			0,
			1
		};
		VkImageViewCreateInfo imageViewCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
			outFramebuffer.depthStencilTexture.image,
			VK_IMAGE_VIEW_TYPE_2D,
			imageCI.format,
			componentMapping,
			imageSubresourceRange
		};
		if (vkCreateImageView(device, &imageViewCI, allocator, &(outFramebuffer.depthStencilView)) != VK_SUCCESS)
		{
			return VK_FALSE;
		}
		return VK_TRUE;
	}

	VkBool32 RenderDevice::InstantiateFramebuffer(Framebuffer& outFramebuffer)
	{
		std::vector<VkImageView> attachments_v;
		for (auto& cv : outFramebuffer.colorViews_v)
		{
			attachments_v.push_back(cv);
		}
		if (outFramebuffer.depthStencilView != VK_NULL_HANDLE)
		{
			attachments_v.push_back(outFramebuffer.depthStencilView);
		}

		VkFramebufferCreateInfo framebufferCI =
		{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0,
			presentRenderPass,
			(uint32_t)attachments_v.size(),
			attachments_v.data(),
			outFramebuffer.size.width,
			outFramebuffer.size.height,
			1
		};

		if (vkCreateFramebuffer(device, &framebufferCI, allocator, &(outFramebuffer.framebuffer)) == VK_SUCCESS)
		{
			return VK_TRUE;
		}
		return VK_FALSE;
	}

	void RenderDevice::DestroyFramebuffer(Framebuffer& fb)
	{
		if (fb.depthStencilView)
		{
			vkDestroyImageView(device, fb.depthStencilView, allocator);
			fb.depthStencilView = VK_NULL_HANDLE;
			DestroyTexture(fb.depthStencilTexture);
		}

		for (size_t cv = 0; cv < fb.colorViews_v.size(); ++cv)
		{
			vkDestroyImageView(device, fb.colorViews_v[cv], allocator);
			if (fb.deleteColorBuffers)
			{
				DestroyTexture(fb.colorTextures_v[cv]);
			}
		}
		fb.colorViews_v.clear();

		vkDestroyFramebuffer(device, fb.framebuffer, allocator);
	}

	VkBool32 RenderDevice::CreateSwapchain(uint32_t numImages)
	{
		// Returns the number of swap chain images, so 0 meeans failure
		VkSurfaceFormatKHR surfaceFormat = DetermineSurfaceFormat(physicalDevice, wndSurface);
		if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
		{
			return VK_FALSE;
		}

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		if (!IsPresentModeSupported(physicalDevice, wndSurface, presentMode))
		{
			return VK_FALSE;
		}

		VkExtent2D wndExtent = {};
		const bhWindowSettings* ws = bhConfig_GetWindowSettings();
		wndExtent.width = ws->width;
		wndExtent.height = ws->height;

		const uint32_t imageArrayLayers = 1;
		const VkSurfaceTransformFlagBitsKHR initTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		const VkSurfaceTransformFlagsKHR reqTransforms = initTransform;
		const VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		surfaceCapabilities.currentExtent = wndExtent;
		surfaceCapabilities.currentTransform = initTransform;
		surfaceCapabilities.maxImageArrayLayers = imageArrayLayers;
		surfaceCapabilities.minImageCount = numImages;
		surfaceCapabilities.supportedCompositeAlpha = compositeAlpha;
		surfaceCapabilities.supportedTransforms = reqTransforms;

		if (!TestPhysicalDeviceSurfaceCaps(physicalDevice, wndSurface, &surfaceCapabilities))
		{
			return VK_FALSE;
		}

		VkSwapchainCreateInfoKHR swapChainCI =
		{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr , 0,
			wndSurface, //surface
			numImages, //minImageCount
			surfaceFormat.format, //imageFormat
			surfaceFormat.colorSpace, //imageColorSpace
			wndExtent, //imageExtent
			imageArrayLayers, //imageArrayLayers
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, //imageUsage
			VK_SHARING_MODE_EXCLUSIVE, //imageSharingMode
			0, //queueFamilyIndexCount
			nullptr, //pQueueFamilyIndices
			initTransform, //preTransform
			compositeAlpha, //compositeAlpha
			presentMode, //presentMode
			VK_TRUE //clipped
			//oldSwapchain
		};

		uint32_t numSwapChainImages = 0;
		if (vkCreateSwapchainKHR(device, &swapChainCI, allocator, &(swapChain.swapChain)) == VK_SUCCESS)
		{
			if (vkGetSwapchainImagesKHR(device, swapChain.swapChain, &numSwapChainImages, nullptr) == VK_SUCCESS)
			{
				std::vector<VkImage> swapChainColorBuffers_v(numSwapChainImages);
				if (vkGetSwapchainImagesKHR(device, swapChain.swapChain, &numSwapChainImages, swapChainColorBuffers_v.data()) == VK_SUCCESS)
				{
					//Setup Render targets
					swapChain.frameBuffers_v.resize(numSwapChainImages);
					for (uint32_t fbIdx = 0; fbIdx < numSwapChainImages; ++fbIdx)
					{
						CreateFramebuffer(swapChain.frameBuffers_v[fbIdx], wndExtent, swapChainColorBuffers_v.data()[fbIdx], surfaceFormat, true);
					}
				}
			}
		}
		return (numSwapChainImages > 0) ? VK_TRUE : VK_FALSE;
	}

	void RenderDevice::DestroySwapchain()
	{
		for (size_t fb = 0; fb < swapChain.frameBuffers_v.size(); ++fb)
		{
			DestroyFramebuffer(swapChain.frameBuffers_v[fb]);
		}
		//g_swapChainImages_v are handled by the swapChain, and destroyed along with it
		vkDestroySwapchainKHR(device, swapChain.swapChain, allocator);
		swapChain.Clear();
	}

	VkBool32 RenderDevice::CreateCommandBuffers(uint32_t numBuffers)
	{
		VkCommandBufferAllocateInfo commandBufferAI =
		{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
			commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			numBuffers
		};

		commandBuffers_v.resize(numBuffers);
		VkResult res = vkAllocateCommandBuffers(device, &commandBufferAI, commandBuffers_v.data());
		return res == VK_SUCCESS ? VK_TRUE : VK_FALSE;
	}

	Texture RenderDevice::CreateTexture(const VkImageCreateInfo& imageCI)
	{
		VkImage newImage = VK_NULL_HANDLE;
		if (vkCreateImage(device, &imageCI, allocator, &newImage) == VK_SUCCESS)
		{
			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(device, newImage, &memoryRequirements);

			VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			uint32_t memTypeIdx = FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
			assert(memTypeIdx < UINT32_MAX);

			VkMemoryAllocateInfo memoryAllocateInfo =
			{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
				memoryRequirements.size, //allocationSize
				memTypeIdx //memoryTypeIndex
			};

			VkDeviceMemory newMemory = VK_NULL_HANDLE;
			if (vkAllocateMemory(device, &memoryAllocateInfo, allocator, &newMemory) == VK_SUCCESS)
			{
				vkBindImageMemory(device, newImage, newMemory, 0);
				return Texture(newImage, newMemory);// , imageCI.mipLevels, imageCI.format);
			}
			vkDestroyImage(device, newImage, allocator);
		}
		return {};
	}

	VkBool32 RenderDevice::CreateShaderStageFromFile(VkPipelineShaderStageCreateInfo& shaderStageCI, const char* fileName, const char* entryPoint) const
	{
		shaderStageCI = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO , nullptr,0 };

		bhShaderType type = bhTypes_GetShaderTypeFromFileExtension(fileName);
		if (type == SHADER_TYPE_UNKNOWN)
		{
			return VK_FALSE;
		}

		const char* path = bhPlatform::CreateResourcePath(bhPlatform::RT_SHADER_BIN_VULKAN, fileName);
		void* fileData = nullptr;
		long fileLen = bhUtil::ReadFile(path, true, fileData);
		if (fileData && fileLen && ((fileLen % 4) == 0 /*SPIRV constraint*/))
		{
			VkShaderModuleCreateInfo shaderModuleCI =
			{
				VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0,
				(size_t)fileLen,
				reinterpret_cast<const uint32_t*>(fileData)
			};
			VkShaderModule newShaderMd = VK_NULL_HANDLE;
			VkResult result = vkCreateShaderModule(device, &shaderModuleCI, allocator, &newShaderMd);
			if (result == VK_SUCCESS)
			{
				shaderStageCI.stage = GetShaderStage(type);
				shaderStageCI.module = newShaderMd;
				shaderStageCI.pName = entryPoint;
				//shaderStageCI.pSpecializationInfo = ;
			}
			bhUtil::FreeFileData(fileData);
			bhPlatform::FreePath(path);
			return VK_TRUE;
		}
		bhLog_Message(bhLogPriority::LP_ERROR, "Error opening shader file for reading: %s", path);
		bhPlatform::FreePath(path);
		return VK_FALSE;
	}

	void RenderDevice::BeginFrame()
	{
		vkAcquireNextImageKHR(device, swapChain.swapChain, UINT64_MAX, sph_ImageAvailable, VK_NULL_HANDLE, &(swapChain.swapChainIdx));

		VkCommandBufferBeginInfo commandBufferBI =
		{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		vkBeginCommandBuffer(commandBuffers_v[swapChain.swapChainIdx], &commandBufferBI);
		//vkCmdClearAttachments(commandBuffers_v[swapChainIdx], config_ClearAttachmentCount, config_ClearAttachments, g_numClearRects, g_clearRects_v);

		// Begin render pass
		VkRect2D renderArea =
		{
			{0, 0},
			swapChain.frameBuffers_v[swapChain.swapChainIdx].size
		};

		VkRenderPassBeginInfo renderPassBI =
		{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr,
			presentRenderPass,
			swapChain.frameBuffers_v[swapChain.swapChainIdx].framebuffer,
			renderArea,
			2, clearValues_v
		};

		vkCmdBeginRenderPass(commandBuffers_v[swapChain.swapChainIdx], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderDevice::EndFrame()
	{
		vkCmdEndRenderPass(commandBuffers_v[swapChain.swapChainIdx]);

		vkEndCommandBuffer(commandBuffers_v[swapChain.swapChainIdx]);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo =
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr,
			1,
			&sph_ImageAvailable,
			waitStages,
			1,
			&(commandBuffers_v[swapChain.swapChainIdx]),
			1,
			&sph_RenderFinished
		};
		vkQueueSubmit(renderQueue, 1, &submitInfo, drawFence);

		vkWaitForFences(device, 1, &drawFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &drawFence);

		VkPresentInfoKHR presentInfo =
		{
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr,
			1,
			&sph_RenderFinished,
			1,
			&(swapChain.swapChain),
			&(swapChain.swapChainIdx)
		};
		vkQueuePresentKHR(renderQueue, &presentInfo);
		++frameCount;
	}

	//VkPipelineLayout RenderDevice::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& CI) const
	//{
	//	VkPipelineLayout layout = VK_NULL_HANDLE;
	//	if (vkCreatePipelineLayout(device, &CI, allocator, &layout) == VK_SUCCESS)
	//	{
	//		return layout;
	//	}
	//	return VK_NULL_HANDLE;
	//}

	VkPipeline RenderDevice::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo) const
	{
		VkPipeline newPipeline = VK_NULL_HANDLE;
		VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, allocator, &newPipeline);
		if (result == VK_SUCCESS)
		{
			return newPipeline;
		}
		// TODO: Log error info?
		return VK_NULL_HANDLE;
	}

	VkPipeline RenderDevice::CreateComputePipeline(const VkComputePipelineCreateInfo& createInfo) const
	{
		VkPipeline newPipeline = VK_NULL_HANDLE;
		VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, allocator, &newPipeline);
		if (result == VK_SUCCESS)
		{
			return newPipeline;
		}
		// TODO: Log error info?
		return VK_NULL_HANDLE;
	}

	void RenderDevice::DestroyPipeline(VkPipeline pipeline) const
	{
		vkDestroyPipeline(device, pipeline, allocator);
	}

	Buffer RenderDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize) const
	{
		VkBufferCreateInfo bufferCreateInfo =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0,
			reqSize,
			usageFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr
		};

		Buffer newBuffer;
		if (vkCreateBuffer(device, &bufferCreateInfo, allocator, &(newBuffer.buffer)) != VK_SUCCESS)
		{
			// TODO: Report error
			return newBuffer;
		}
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, newBuffer.buffer, &memoryRequirements);

		//VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		uint32_t memTypeIdx = FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, propertyFlags);
		if (memTypeIdx < UINT32_MAX)
		{
			VkMemoryAllocateInfo memoryAllocateInfo =
			{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
				memoryRequirements.size,
				memTypeIdx
			};

			if (vkAllocateMemory(device, &memoryAllocateInfo, allocator, &(newBuffer.memory)) == VK_SUCCESS)
			{
				newBuffer.size = memoryRequirements.size;
				vkBindBufferMemory(device, newBuffer.buffer, newBuffer.memory, 0); // TODO: Error check
				return newBuffer;
			}
		}

		DestroyBuffer(newBuffer);
		return newBuffer;
	}

	void RenderDevice::DestroyBuffer(Buffer& buffer) const
	{
		vkFreeMemory(device, buffer.memory, allocator);
		vkDestroyBuffer(device, buffer.buffer, allocator);
		buffer = {};
	}
	
	VkBool32 RenderDevice::CopyDataToBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data) const
	{
		assert(buffer.memory != VK_NULL_HANDLE);
		if (offset + reqSize > buffer.size)
		{
			bhLog_Message(bhLogPriority::LP_CRITICAL, "CopyDataToBuffer : Exceeding buffer size!");
			return VK_FALSE;
		}

		void* memory = nullptr;
		if (vkMapMemory(device, buffer.memory, offset, reqSize, 0, &memory) == VK_SUCCESS)
		{
			memcpy(memory, data, reqSize);
			//{
			//	VkMappedMemoryRange mmr = {};
			//	mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			//	mmr.memory = deviceMemory;
			//	mmr.offset = offset;
			//	mmr.size = reqSize;
			//	vkFlushMappedMemoryRanges(device, 1, &mmr);
			//}
			vkUnmapMemory(device, buffer.memory);
			return VK_TRUE;
		}
		return VK_FALSE;
	}

	Texture RenderDevice::CreateMemoryBackedImage(const VkImageCreateInfo* imageCI) const
	{
		VkImage newImage = VK_NULL_HANDLE;
		if (vkCreateImage(device, imageCI, allocator, &newImage) != VK_SUCCESS)
		{
			return {};
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, newImage, &memoryRequirements);

		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		uint32_t memTypeIdx = FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
		assert(memTypeIdx < UINT32_MAX);

		VkMemoryAllocateInfo memoryAllocateInfo =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
			memoryRequirements.size, //allocationSize
			memTypeIdx //memoryTypeIndex
		};

		VkDeviceMemory newMemory = VK_NULL_HANDLE;
		if (vkAllocateMemory(device, &memoryAllocateInfo, allocator, &newMemory) == VK_SUCCESS)
		{
			vkBindImageMemory(device, newImage, newMemory, 0);
			return Texture(newImage, newMemory);
		}
		vkDestroyImage(device, newImage, allocator);

		return {};
	}

	Texture RenderDevice::CreateTexture(VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels) const
	{
		VkImageCreateInfo imageCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
			imageType, //imageType
			fmt, //format
			extent, //extent
			mipLevels, //mipLevels
			1, //arrayLayers
			VK_SAMPLE_COUNT_1_BIT, //samples
			VK_IMAGE_TILING_OPTIMAL, //tiling
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, //usage
			VK_SHARING_MODE_EXCLUSIVE, //sharingMode
			0, //queueFamilyIndexCount
			nullptr, //pQueueFamilyIndices
			VK_IMAGE_LAYOUT_UNDEFINED //initialLayout
		};

		return CreateMemoryBackedImage(&imageCI);
	}

	void RenderDevice::DestroyTexture(Texture& texture) const
	{
		vkFreeMemory(device, texture.memory, allocator);
		vkDestroyImage(device, texture.image, allocator);
		texture = {};
	}

	bhVk::Buffer RenderDevice::CreateBufferFromImage(const bhImage* img) const
	{
		VkDeviceSize imageSize = bhImage_GetMemSiz(img);

		VkBufferCreateInfo bufferCreateInfo =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr
		};

		bhVk::Buffer newBuffer;
		if (vkCreateBuffer(device, &bufferCreateInfo, allocator, &(newBuffer.buffer)) != VK_SUCCESS)
		{
			return {};
		}

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, newBuffer.buffer, &memoryRequirements);

		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		uint32_t memTypeIdx = FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
		assert(memTypeIdx < UINT32_MAX);

		VkMemoryAllocateInfo memoryAllocateInfo =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
				memoryRequirements.size,
				memTypeIdx
		};

		if (vkAllocateMemory(device, &memoryAllocateInfo, allocator, &(newBuffer.memory)) != VK_SUCCESS)
		{
			vkDestroyBuffer(device, newBuffer.buffer, allocator);
			return {};
		}

		newBuffer.size = memoryRequirements.size;
		vkBindBufferMemory(device, newBuffer.buffer, newBuffer.memory, 0);
		void* data;
		vkMapMemory(device, newBuffer.memory, 0, imageSize, 0, &data);
		memcpy(data, img->pixels, imageSize);
		vkUnmapMemory(device, newBuffer.memory);

		return newBuffer;
	}

	Texture RenderDevice::CreateTextureFromImage(const bhImage* img, int32_t offsetx, int32_t offsety) const
	{
		// Try to make sense out of img components
		VkFormat fmt = PickImageFormat(img);
		if (fmt == VK_FORMAT_UNDEFINED)
		{
			return {};
		}

		VkExtent3D extent = {};
		extent.width = img->width;
		extent.height = img->height;
		extent.depth = img->depth;

		// Create the empty texture with memory
		uint32_t requiredMipLevels = bhImage_GetRequiredMipLevels(img);
		Texture newTex = CreateTexture(MapImageTypeToVk(bhImage_DetermineDimensions(img)), fmt, extent, requiredMipLevels);
		if (!newTex.IsValid())
		{
			return {};
		}

		// Create the buffer from pixel data to copy the texture img from
		bhVk::Buffer imageBuffer = CreateBufferFromImage(img);
		if (!imageBuffer.IsValid())
		{
			return {};
		}

		VkCommandBuffer commandBuffer = bhVk::BeginSingleTimeCommandBuffer(device, commandPool);
		if (commandBuffer != VK_NULL_HANDLE)
		{
			VkOffset3D offset = { offsetx, offsety, 0 };

			VkImageSubresourceRange imageSubresourceRange =
			{
				VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
				0, //baseMipLevel
				requiredMipLevels, //levelCount
				0, //baseArrayLayer
				1 //layerCount
			};

			VkImageMemoryBarrier imageMemoryBarrier =
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
				0, //srcAccessMask
				VK_ACCESS_TRANSFER_WRITE_BIT, //dstAccessMask
				VK_IMAGE_LAYOUT_UNDEFINED, //oldLayout
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //newLayout
				VK_QUEUE_FAMILY_IGNORED, //srcQueueFamilyIndex
				VK_QUEUE_FAMILY_IGNORED, //dstQueueFamilyIndex
				newTex.image, //image
				imageSubresourceRange //subresourceRange
			};
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			VkImageSubresourceLayers subresourceLayers =
			{
				VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
				0, //mipLevel
				0, //baseArrayLayer
				1 //layerCount
			};

			VkBufferImageCopy bufferImageCopy =
			{
				0, //bufferOffset
				0, //bufferRowLength
				0, //bufferImageHeight
				subresourceLayers, //imageSubresource
				offset, //imageOffset
				extent //extent
			};
			vkCmdCopyBufferToImage(commandBuffer, imageBuffer.buffer, newTex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

			const bhRenderSettings* rs = bhConfig_GetRenderSettings();
			if (bhConfig_GenMipmaps(rs))
			{
				GenerateMipmaps(commandBuffer, img, requiredMipLevels, newTex);
			}

			////////////////////////////////////////////////////////////////////////////////
			// Make img shader readable
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.oldLayout = (bhConfig_GenMipmaps(rs) != 0) ?
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL :
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			bhVk::EndSingleTimeCommandBuffer(device, commandPool, commandBuffer, renderQueue);
		}
		// Clear intermediate img source buffer
		DestroyBuffer(imageBuffer);
		return newTex;
	}

	VkDescriptorImageInfo RenderDevice::SetupTextureDescriptor(const bhTexture& tx) const
	{
		VkDescriptorImageInfo descriptorImgInfo = {};
		descriptorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImgInfo.imageView = CreateTextureImageView(tx);

		float reqAnisotropy = GetAnisotropy();
		VkSamplerCreateInfo samplerCI =
		{
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0,
			VK_FILTER_LINEAR, //magFilter
			VK_FILTER_LINEAR, //minFilter
			VK_SAMPLER_MIPMAP_MODE_LINEAR, //mipmapMode
			VK_SAMPLER_ADDRESS_MODE_REPEAT, //addressModeU
			VK_SAMPLER_ADDRESS_MODE_REPEAT, //addressModeV
			VK_SAMPLER_ADDRESS_MODE_REPEAT, //addressModeW
			0.0f, //mipLodBias
			(reqAnisotropy > 0.0f) ? VK_TRUE : VK_FALSE, //anisotropyEnable
			reqAnisotropy, //maxAnisotropy
			VK_FALSE, //compareEnable
			VK_COMPARE_OP_ALWAYS, //compareOp
			0.0f, //minLod
			FLT_MAX, //maxLod
			VK_BORDER_COLOR_INT_OPAQUE_BLACK, //borderColor
			VK_FALSE //unnormalizedCoordinates
		};

		VkResult res = vkCreateSampler(device, &samplerCI, allocator, &descriptorImgInfo.sampler);
		if (res != VK_SUCCESS)
		{
			// Report VK_ERROR_OUT_OF_HOST_MEMORY / VK_ERROR_OUT_OF_DEVICE_MEMORY
		}
		return descriptorImgInfo;
	}

	VkImageView RenderDevice::CreateTextureImageView(const bhTexture& tx) const
	{
		VkFormat fmt = PickImageFormat(tx.GetImage());
		if (fmt == VK_FORMAT_UNDEFINED)
		{
			return VK_NULL_HANDLE;
		}

		VkComponentMapping componentMapping = {};

		VkImageSubresourceRange imageSubresourceRange =
		{
			VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
			0, //baseMipLevel
			bhImage_GetRequiredMipLevels(tx.GetImage()), //levelCount
			0, //baseArrayLayer
			1 //layerCount
		};

		VkImageViewCreateInfo imageViewCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
			tx.GetTexture().image,
			VK_IMAGE_VIEW_TYPE_2D,
			fmt,
			componentMapping,
			imageSubresourceRange
		};

		VkImageView imageView = VK_NULL_HANDLE;
		vkCreateImageView(device, &imageViewCI, allocator, &imageView);
		return imageView;
	}

	float RenderDevice::GetAnisotropy() const
	{
		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		if (physicalDeviceFeatures.samplerAnisotropy)
		{
			const bhRenderSettings* rs = bhConfig_GetRenderSettings();
			return (rs->anisotropy_level == 0) ? 0.0f : float(1 << rs->anisotropy_level);
		}
		return 0.0f;
	}

	//////////////////////////////////////////////////////////////////////////////////

	static void check_vk_result(VkResult result)
	{
		if (result == VK_SUCCESS)
		{
			return;
		}
		bhLog_Message(bhLogPriority::LP_ERROR, "[vulkan] Error: VkResult = %d\n", result);
		assert(!result);
	}

	VkBool32 RenderDevice::InitImGui(SDL_Window* wnd) const
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		if (ImGui_ImplSDL2_InitForVulkan(wnd))
		{
			const bhRenderSettings* rs = bhConfig_GetRenderSettings();
			static const size_t VERSION_SRING_LEN = 16;
			char versionString[VERSION_SRING_LEN];
			sprintf_s(versionString, VERSION_SRING_LEN, "#version %d%d0", rs->gl.versionMajor, rs->gl.versionMinor);

			ImGui_ImplVulkan_InitInfo initInfo = {};
			initInfo.Instance = instance.GetInstance();
			initInfo.PhysicalDevice = physicalDevice;
			initInfo.Device = device;
			initInfo.QueueFamily = gfxQueueFamilyIdx;
			initInfo.Queue = renderQueue;
			//initInfo.PipelineCache = g_PipelineCache;
			initInfo.DescriptorPool = descriptorPool;
			initInfo.Subpass = 0;
			initInfo.MinImageCount = 2;
			initInfo.ImageCount = static_cast<uint32_t>(swapChain.frameBuffers_v.size());
			initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			initInfo.Allocator = allocator;
			initInfo.CheckVkResultFn = check_vk_result;

			ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
				return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
				}, instance.GetInstancePtr());
			ImGui_ImplVulkan_Init(&initInfo, presentRenderPass);

			// Upload Fonts
			{
				// Use any command queue
				VkCommandBuffer command_buffer = commandBuffers_v[0];
				check_vk_result(vkResetCommandPool(device, commandPool, 0));

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				check_vk_result(vkBeginCommandBuffer(command_buffer, &beginInfo));
				ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

				VkSubmitInfo end_info = {};
				end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				end_info.commandBufferCount = 1;
				end_info.pCommandBuffers = &command_buffer;
				check_vk_result(vkEndCommandBuffer(command_buffer));
				check_vk_result(vkQueueSubmit(renderQueue, 1, &end_info, VK_NULL_HANDLE));

				check_vk_result(vkDeviceWaitIdle(device));
				ImGui_ImplVulkan_DestroyFontUploadObjects();
			}
			return VK_TRUE;
		}
		return VK_FALSE;
	}

	void RenderDevice::DestroyImGui() const
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}

	void RenderDevice::BeginImGuiFrame() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
	}

	void RenderDevice::EndImGuiFrame() const
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers_v[swapChain.swapChainIdx]);
	}
}
