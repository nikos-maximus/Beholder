#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN

#include "SDL_vulkan.h"
#include "VK/bhDeviceVK.hpp"
#include "bhWindow.h"
#include "bhConfig.h"
#include "bhEnv.h"
#include "bhImage.h"
#include "bhUtil.h"

#ifdef BH_USE_IMGUI
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"
#endif //BH_USE_IMGUI

////////////////////////////////////////////////////////////////////////////////
uint32_t bhDeviceVK::RankPhysicalDeviceGraphicsFeatures(const VkPhysicalDevice& physicalDevice)
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

VkBool32 bhDeviceVK::CreateDescriptorPool()
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
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0,
		maxSets,
		(uint32_t)descriptorPoolSizes_v.size(),
		descriptorPoolSizes_v.data()
	};

	VkResult result = vkCreateDescriptorPool(device, &descriptorPoolCI, deviceAllocator, &descriptorPool);
	return (result == VK_SUCCESS) ? VK_TRUE : VK_FALSE;
}

bool bhDeviceVK::Init()
{
	instanceVK = bhVK_InitInstance();
	if (instanceVK.instance == VK_NULL_HANDLE)
	{
		return false;
	}
	
#ifdef BH_API_DEBUG
	VkBool32 cbResult = bhVK_InitDebugCallback(&instanceVK);
	assert(cbResult == VK_TRUE);
#endif //BH_API_DEBUG
	
	bhImage_Init();
	bhEnv_SetApplicationName("Beholder (Vulkan)");
	if (bhCreateMainWindow() == 0)
	{
		if (SDL_Vulkan_CreateSurface(bhGetMainWindow(), instanceVK.instance, &windowSurface) == SDL_FALSE)
		{
			return false;
		}
		if (CreateRenderDevice())
		{
			const bhRenderSettings* rs = bhConfig_GetRenderSettings();

			renderPass = CreateRenderPass();
			CreateSwapchain(&swapChain, rs->num_swapchain_images);

			assert(rs->num_swapchain_images == swapChain.frameBuffers_v.size());
			commandBuffers_v = bhVK_CreateCommandBuffers(device, commandPool, rs->num_swapchain_images);
			vkGetDeviceQueue(device, gfxQueueFamilyIdx, 0, &renderQueue);

			drawFence = bhVK_CreateFence(device, deviceAllocator);

			// Create semaphores
			VkSemaphoreCreateInfo semaphoreCI =
			{
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};
			vkCreateSemaphore(device, &semaphoreCI, deviceAllocator, &sph_ImageAvailable);
			vkCreateSemaphore(device, &semaphoreCI, deviceAllocator, &sph_RenderFinished);

			//g_clearRects_v[0] = {};
			//g_clearRects_v[0].rect.extent.width = bhConfig::GetWidth();
			//g_clearRects_v[0].rect.extent.height = bhConfig::GetHeight();

			return true;
		}
	}
	return false;
}

void bhDeviceVK::Destroy()
{
	textureManager.DestroyAll([this](bhTextureVK* t)
		{
			this->DestroyTexture(t);
		});

	DestroySwapchain(&swapChain);
	bhVK_DestroyCommandBuffers(&commandBuffers_v, device, commandPool);

	vkDestroyRenderPass(device, renderPass, deviceAllocator);

	bhVK_DestroyFence(drawFence, device, deviceAllocator);
	
	// Destroy semaphores
	vkDestroySemaphore(device, sph_RenderFinished, deviceAllocator);
	vkDestroySemaphore(device, sph_ImageAvailable, deviceAllocator);

	vkDeviceWaitIdle(device);
	vkDestroyDescriptorPool(device, descriptorPool, deviceAllocator);
	vkDestroyCommandPool(device, commandPool, deviceAllocator);
	vkDestroyDevice(device, deviceAllocator);

	vkDestroySurfaceKHR(instanceVK.instance, windowSurface, instanceVK.allocator);
	windowSurface = VK_NULL_HANDLE;

	bhDestroyMainWindow();
	
#ifdef BH_API_DEBUG
	bhVK_DestroyDebugCallback(&instanceVK);
#endif //BH_API_DEBUG

	bhVK_DestroyInstance(&instanceVK);
}

void bhDeviceVK::BeginFrame()
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
		renderPass,
		swapChain.frameBuffers_v[swapChain.swapChainIdx].framebuffer,
		renderArea,
		(uint32_t)clearValues_v.size(),
		clearValues_v.data()
	};

	vkCmdBeginRenderPass(commandBuffers_v[swapChain.swapChainIdx], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
}

void bhDeviceVK::EndFrame()
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
		& (commandBuffers_v[swapChain.swapChainIdx]),
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
	//++frameCount;
}

VkImageType bhDeviceVK::MapImageTypeToVulkan(int dim)
{
	if ((0 < dim) && (dim < 4))
	{
		return (VkImageType)(dim - 1);
	}
	return VK_IMAGE_TYPE_MAX_ENUM; // Should be treated as invalid
}

void bhDeviceVK::GenerateMipmaps(VkCommandBuffer commandBuffer, const bhImage* img, uint32_t numMipLevels, bhTextureVK* texture)
{
	int w = img->width;
	int h = img->height;

	// Define the common parts of VkImageBlit
	VkImageBlit imageBlit = {};
	imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.srcSubresource.layerCount = 1;

	imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.dstSubresource.layerCount = 1;

	VkImageSubresourceRange imageSubresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
		0, //baseMipLevel
		1, //levelCount
		0, //baseArrayLayer
		1 //layerCount
	};

	// Make (existing) mip level 0 transfer readable
	VkImageMemoryBarrier imageMemoryBarrier =
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
		0, //srcAccessMask
		VK_ACCESS_TRANSFER_WRITE_BIT, //dstAccessMask
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, //oldLayout
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, //newLayout
		VK_QUEUE_FAMILY_IGNORED, //srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED, //dstQueueFamilyIndex
		texture->image, //image
		imageSubresourceRange //subresourceRange
	};
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	for (uint32_t level = 1; level < numMipLevels; ++level)
	{
		// Make mip level writable
		imageMemoryBarrier.subresourceRange.baseMipLevel = level;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		imageBlit.srcSubresource.mipLevel = level - 1;
		imageBlit.srcOffsets[1].x = w;
		imageBlit.srcOffsets[1].y = h;
		imageBlit.srcOffsets[1].z = 1;

		w /= 2;
		h /= 2;

		imageBlit.dstSubresource.mipLevel = level;
		imageBlit.dstOffsets[1].x = w;
		imageBlit.dstOffsets[1].y = h;
		imageBlit.dstOffsets[1].z = 1;

		// Copy mip level data from previous mip level
		vkCmdBlitImage(commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		// Make mip level readable for next blit iteration
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}
	//imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

bhTextureVK* bhDeviceVK::CreateTextureFromImage_Local(const bhImage* img, int32_t offsetx, int32_t offsety)
{
	// Try to make sense out of img components
	VkFormat fmt = PickImageFormat(img);
	if (fmt == VK_FORMAT_UNDEFINED)
	{
		return nullptr;
	}

	VkExtent3D extent = {};
	extent.width = img->width;
	extent.height = img->height;
	extent.depth = img->depth;

	// Create the empty texture with memory
	uint32_t requiredMipLevels = bhImage_GetRequiredMipLevels(img);
	bhTextureVK* newTexture = CreateTexture(MapImageTypeToVulkan(bhImage_DetermineDimensions(img)), fmt, extent, requiredMipLevels);
	if (!newTexture)
	{
		return nullptr;
	}

	// Create the buffer from pixel data to copy the texture img from
	bhBufferVK imageBuffer = CreateBufferFromImage(img);
	if (!bhBufferVK_IsValid(&imageBuffer))
	{
		return nullptr;
	}

	VkCommandBuffer commandBuffer = bhVK_BeginSingleTimeCommandBuffer(device, commandPool);
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
			newTexture->image, //image
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
		vkCmdCopyBufferToImage(commandBuffer, imageBuffer.buffer, newTexture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

		const bhRenderSettings* rs = bhConfig_GetRenderSettings();
		if (bhConfig_GenMipmaps(rs))
		{
			GenerateMipmaps(commandBuffer, img, requiredMipLevels, newTexture);
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

		bhVK_EndSingleTimeCommandBuffer(device, commandPool, commandBuffer, renderQueue);
	}
	// Clear intermediate img source buffer
	DestroyBuffer(&imageBuffer);
	return newTexture;
}

bhResourceID bhDeviceVK::CreateTextureFromImage(const bhImage* img, const char* name, int32_t offsetx, int32_t offsety)
{
	// Check if we already created the texture
	bhResourceID textureID = textureManager.Find(name);
	if (textureID)
	{
		return textureID;
	}

	bhTextureVK* texture = CreateTextureFromImage_Local(img, offsetx, offsety);
	return textureManager.AddNamedResource(name, texture);
}

VkFormat bhDeviceVK::PickImageFormat(const bhImage* img)
{
	switch (img->numComponents)
	{
		case 1:
		{
			return VK_FORMAT_R8_UNORM;
		}
		case 2:
		{
			return VK_FORMAT_R8G8_UNORM;
		}
		case 3:
		{
			return VK_FORMAT_R8G8B8_UNORM;
		}
		case 4:
		{
			return VK_FORMAT_R8G8B8A8_UNORM;
		}
		default:
		{
			return VK_FORMAT_UNDEFINED;

		}
	}
}

VkRenderPass bhDeviceVK::CreateRenderPass()
{
	std::vector<VkAttachmentDescription> attachmentDescriptions_v(2);
	{
		attachmentDescriptions_v[0].flags = 0;
		attachmentDescriptions_v[0].format = bhVK_DetermineSurfaceFormat(physicalDevice, windowSurface).format;
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

	VkRenderPass newRenderPass = VK_NULL_HANDLE;
	if (vkCreateRenderPass(device, &renderPassCI, deviceAllocator, &newRenderPass) == VK_SUCCESS)
	{
		clearValues_v[0].color.float32[0] = 1.f;
		clearValues_v[0].color.float32[1] = 0.f;
		clearValues_v[0].color.float32[2] = 1.f;
		clearValues_v[0].color.float32[3] = 1.f;

		clearValues_v[1].depthStencil.depth = 1.f;
		clearValues_v[1].depthStencil.stencil = 0;
	}
	return newRenderPass;
}

void bhDeviceVK::ClearSwapchain(bhSwapchainVK* bhSwapchain)
{
	bhSwapchain->frameBuffers_v.clear();
	bhSwapchain->swapChainIdx = UINT32_MAX;
	bhSwapchain->swapChain = VK_NULL_HANDLE;
}

uint32_t bhDeviceVK::CreateSwapchain(bhSwapchainVK* outSwapchain, uint32_t numImages)
{
	// Returns the number of swap chain images, so 0 meeans failure
	assert(windowSurface != VK_NULL_HANDLE);

	VkSurfaceFormatKHR surfaceFormat = bhVK_DetermineSurfaceFormat(physicalDevice, windowSurface);
	if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
	{
		return 0;
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!bhVK_IsPresentModeSupported(physicalDevice, windowSurface, presentMode))
	{
		return 0;
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

	if (!bhVK_TestPhysicalDeviceSurfaceCaps(physicalDevice, windowSurface, &surfaceCapabilities))
	{
		return false;
	}

	VkSwapchainCreateInfoKHR swapChainCI =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr , 0,
		windowSurface, //surface
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

	if (vkCreateSwapchainKHR(device, &swapChainCI, deviceAllocator, &(outSwapchain->swapChain)) == VK_SUCCESS)
	{
		uint32_t numSwapChainImages = 0;
		if (vkGetSwapchainImagesKHR(device, outSwapchain->swapChain, &numSwapChainImages, nullptr) == VK_SUCCESS)
		{
			std::vector<VkImage> swapChainColorBuffers_v(numSwapChainImages);
			if (vkGetSwapchainImagesKHR(device, outSwapchain->swapChain, &numSwapChainImages, swapChainColorBuffers_v.data()) == VK_SUCCESS)
			{
				//Setup Render targets
				outSwapchain->frameBuffers_v.resize(numSwapChainImages);
				for (uint32_t fbIdx = 0; fbIdx < numSwapChainImages; ++fbIdx)
				{
					bhFramebufferVK* fb = static_cast<bhFramebufferVK*>(CreateFramebuffer(wndExtent, swapChainColorBuffers_v.data()[fbIdx], surfaceFormat, true));
					swapChain.frameBuffers_v[fbIdx] = *fb;
						
				}
				return numSwapChainImages;
			}
		}
	}
	ClearSwapchain(outSwapchain);
	return 0;
}

void bhDeviceVK::DestroySwapchain(bhSwapchainVK* bhSwapchain)
{
	for (size_t fb = 0; fb < bhSwapchain->frameBuffers_v.size(); ++fb)
	{
		DestroyFramebuffer(&(bhSwapchain->frameBuffers_v[fb]));
	}
	//g_swapChainImages_v are handled by the swapChain, and destroyed along with it
	vkDestroySwapchainKHR(device, bhSwapchain->swapChain, deviceAllocator);
	ClearSwapchain(bhSwapchain);
}

VkExtent2D bhDeviceVK::GetMaxFramebufferSize(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	return VkExtent2D{ physicalDeviceProperties.limits.maxFramebufferWidth ,physicalDeviceProperties.limits.maxFramebufferHeight };
}

bool bhDeviceVK::ChoosePhysicalRenderDevice(const VkQueueFamilyProperties* reqQFP_Graphics, const std::vector<const char*>* reqExtNames_v)
{
	assert(instanceVK.instance != VK_NULL_HANDLE);

	uint32_t numPhysicalDevices = 0;
	if (vkEnumeratePhysicalDevices(instanceVK.instance, &numPhysicalDevices, nullptr) != VK_SUCCESS)
	{
		// TODO: Log Error?
		return false;
	}
	std::vector<VkPhysicalDevice> physicalDevices_v(numPhysicalDevices);
	if (vkEnumeratePhysicalDevices(instanceVK.instance, &numPhysicalDevices, physicalDevices_v.data()) != VK_SUCCESS)
	{
		// TODO: Log Error?
		return false;
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

		queueFamilyInds_v[i] = bhVK_GetQueueFamily(physicalDevices_v[i], reqQFP_Graphics, windowSurface);
		if (queueFamilyInds_v[i] == UINT32_MAX)
		{
			continue;
		}
		if (bhVK_TestPhysicalDeviceExtensions(physicalDevices_v[i], reqExtNames_v, nullptr) == false)
		{
			continue;
		}

		ranks_v[i] += RankPhysicalDeviceGraphicsFeatures(physicalDevices_v[i]);
		ranks_v[i] += bhVK_GetDeviceTypeRank(pdp.deviceType);
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
		bhVK_LogPhysicalDevice(physicalDevice);
		return true;
	}
	return false;
}

bool bhDeviceVK::CreateRenderDevice()
{
	VkQueueFamilyProperties reqQFP_Graphics = {};
	reqQFP_Graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT;
	reqQFP_Graphics.queueCount = 1;

	std::vector<const char*> reqExtNames_v(2);
	reqExtNames_v[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	reqExtNames_v[1] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;

	if (!ChoosePhysicalRenderDevice(&reqQFP_Graphics, &reqExtNames_v))
	{
		return false;
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
	//TODO - we should provide only the required features

	const bhRenderSettings* rs = bhConfig_GetRenderSettings();
	physicalDeviceFeatures.samplerAnisotropy = (rs->anisotropy_level > 0);

	std::vector<float> queuePriorities_v(reqQFP_Graphics.queueCount, 1.f);

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

	if (vkCreateDevice(physicalDevice, &deviceCI, instanceVK.allocator, &device) == VK_SUCCESS)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo =
		{
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			gfxQueueFamilyIdx
		};

		VkResult createCPres = vkCreateCommandPool(device, &commandPoolCreateInfo, deviceAllocator, &commandPool);
		assert(createCPres == VK_SUCCESS);

		VkBool32 descriptorPoolResult = CreateDescriptorPool();
		assert(descriptorPoolResult);
	}
	return true;
}

float bhDeviceVK::GetAnisotropy() const
{
	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
	if (physicalDeviceFeatures.samplerAnisotropy)
	{
		const bhRenderSettings* rs = bhConfig_GetRenderSettings();
		return (rs->anisotropy_level == 0) ? 0.f : float(1 << rs->anisotropy_level);
	}
	return 0.f;
}

bhTextureVK* bhDeviceVK::CreateMemoryBackedImage(const VkImageCreateInfo* imageCI)
{
	VkImage newImage = VK_NULL_HANDLE;
	if (vkCreateImage(device, imageCI, deviceAllocator, &newImage) == VK_SUCCESS)
	{
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, newImage, &memoryRequirements);

		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		uint32_t memTypeIdx = bhVK_FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
		assert(memTypeIdx < UINT32_MAX);

		VkMemoryAllocateInfo memoryAllocateInfo =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
			memoryRequirements.size, //allocationSize
			memTypeIdx //memoryTypeIndex
		};

		VkDeviceMemory newMemory = VK_NULL_HANDLE;
		if (vkAllocateMemory(device, &memoryAllocateInfo, deviceAllocator, &newMemory) == VK_SUCCESS)
		{
			vkBindImageMemory(device, newImage, newMemory, 0);
			return new bhTextureVK(newImage, newMemory);
		}
		vkDestroyImage(device, newImage, deviceAllocator);
	}
	return nullptr;
}

bhTextureVK* bhDeviceVK::CreateTexture(VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels)
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

void bhDeviceVK::DestroyTexture(bhTextureVK* texture)
{
	vkFreeMemory(device, texture->memory, deviceAllocator);
	texture->memory = VK_NULL_HANDLE;
	vkDestroyImage(device, texture->image, deviceAllocator);
	texture->image = VK_NULL_HANDLE;
}

bhBufferVK bhDeviceVK::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize)
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

	bhBufferVK newBuffer;
	if (vkCreateBuffer(device, &bufferCreateInfo, deviceAllocator, &(newBuffer.buffer)) == VK_SUCCESS)
	{
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, newBuffer.buffer, &memoryRequirements);

		//VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		uint32_t memTypeIdx = bhVK_FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, propertyFlags);
		if (memTypeIdx < UINT32_MAX)
		{
			VkMemoryAllocateInfo memoryAllocateInfo =
			{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
				memoryRequirements.size,
				memTypeIdx
			};

			if (vkAllocateMemory(device, &memoryAllocateInfo, deviceAllocator, &(newBuffer.memory)) == VK_SUCCESS)
			{
				newBuffer.size = memoryRequirements.size;
				vkBindBufferMemory(device, newBuffer.buffer, newBuffer.memory, 0); // TODO: Error check
				return newBuffer;
			}
		}
	}
	DestroyBuffer(&newBuffer);
	return newBuffer;
}

bhBufferVK bhDeviceVK::CreateBufferFromImage(const bhImage* img)
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

	bhBufferVK newBuffer;
	if (vkCreateBuffer(device, &bufferCreateInfo, deviceAllocator, &(newBuffer.buffer)) == VK_SUCCESS)
	{
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device, newBuffer.buffer, &memoryRequirements);

		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		uint32_t memTypeIdx = bhVK_FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
		assert(memTypeIdx < UINT32_MAX);

		VkMemoryAllocateInfo memoryAllocateInfo =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
				memoryRequirements.size,
				memTypeIdx
		};

		if (vkAllocateMemory(device, &memoryAllocateInfo, deviceAllocator, &(newBuffer.memory)) == VK_SUCCESS)
		{
			newBuffer.size = memoryRequirements.size;
			vkBindBufferMemory(device, newBuffer.buffer, newBuffer.memory, 0);
			void* data;
			vkMapMemory(device, newBuffer.memory, 0, imageSize, 0, &data);
			memcpy(data, img->pixels, imageSize);
			vkUnmapMemory(device, newBuffer.memory);
		}
		else
		{
			vkDestroyBuffer(device, newBuffer.buffer, deviceAllocator);
			newBuffer = {};
		}
	}
	return newBuffer;
}

void bhDeviceVK::DestroyBuffer(bhBufferVK* buffer)
{
	vkFreeMemory(device, buffer->memory, deviceAllocator);
	vkDestroyBuffer(device, buffer->buffer, deviceAllocator);
	*buffer = {};
}

VkBool32 bhDeviceVK::CopyDataToBuffer(const bhBufferVK* buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data)
{
	assert(buffer->memory != VK_NULL_HANDLE);
	if (offset + reqSize <= buffer->size)
	{
		void* memory = nullptr;
		if (vkMapMemory(device, buffer->memory, offset, reqSize, 0, &memory) == VK_SUCCESS)
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
			vkUnmapMemory(device, buffer->memory);
			return VK_TRUE;
		}
	}
	bhLog_Message(LOG_TYPE_CRITICAL, "CopyDataToBuffer : Exceeding buffer size!");
	return VK_FALSE;
}

bool bhDeviceVK::CreateShaderFromFile(const char* fileName, VkShaderModule* newShaderModule, VkShaderStageFlagBits* newShaderType)
{
	char* path = bhUtil_CreatePath(bhEnv_GetEnvString(ENV_SHADERS_BIN_VULKAN_PATH), fileName);
	bhShaderType type = bhTypes_GetShaderTypeFromFileExtension(path);
	if (type == SHADER_TYPE_UNKNOWN)
	{
		return false;
	}

	FileReadData fileData = bhUtil_ReadFile(path, 1);
	bhUtil_FreePath(&path);
	if (fileData.data && ((fileData.length % 4) == 0)) // SPIR-V constraint
	{
		VkShaderModuleCreateInfo shaderModuleCI =
		{
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0,
			(size_t)fileData.length,
			reinterpret_cast<const uint32_t*>(fileData.data)
		};

		if (vkCreateShaderModule(device, &shaderModuleCI, deviceAllocator, newShaderModule) == VK_SUCCESS)
		{
			*newShaderType = bhVK_GetShaderType(type);
			bhUtil_FreeFileData(&fileData);
			return true;
		}
	}
	bhLog_Message(LOG_TYPE_ERROR, "Could not read shader file %s", path);
	return false;
}

VkSampler bhDeviceVK::CreateSampler(const VkSamplerCreateInfo* createInfo) const
{
	VkSampler newSampler = VK_NULL_HANDLE;
	VkResult res = vkCreateSampler(device, createInfo, deviceAllocator, &newSampler);
	if (res != VK_SUCCESS)
	{
		// Report VK_ERROR_OUT_OF_HOST_MEMORY / VK_ERROR_OUT_OF_DEVICE_MEMORY
	}
	return newSampler;
}

void bhDeviceVK::DestroySampler(VkSampler sampler)
{
	vkDestroySampler(device, sampler, deviceAllocator);
}

#ifdef BH_USE_IMGUI

bool bhDeviceVK::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	if (ImGui_ImplSDL2_InitForVulkan(bhGetMainWindow()))
	{
		const bhRenderSettings* rs = bhConfig_GetRenderSettings();
		static const size_t VERSION_SRING_LEN = 16;
		char versionString[VERSION_SRING_LEN];
		sprintf_s(versionString, VERSION_SRING_LEN, "#version %d%d0", rs->gl.versionMajor, rs->gl.versionMinor);
		//if (ImGui_ImplVulkan_Init())
		//{
		//	ImGui::StyleColorsDark();
		//	return true;
		//}
	}
	return false;
}

void bhDeviceVK::DestroyImGui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void bhDeviceVK::BeginImGuiFrame()
{
	ImGui_ImplSDL2_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
}

void bhDeviceVK::EndImGuiFrame()
{
	ImGui::Render();
	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());
}

#endif //BH_USE_IMGUI
