#include "VK/bhDeviceVK.hpp"

void bhDeviceVK::CreateFramebuffer_Common(bhFramebufferVK* outFramebuffer, bool createDepthStencil)
{
	// DepthStencil
	if (createDepthStencil)
	{
		VkImageCreateInfo imageCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_D24_UNORM_S8_UINT,
			{ outFramebuffer->size.width, outFramebuffer->size.height, 1 },
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
			bhTextureVK* tmpTexture = CreateMemoryBackedImage(&imageCI);
			assert(tmpTexture != nullptr);
			outFramebuffer->depthStencilBuffer = *tmpTexture;
			delete tmpTexture;
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
			outFramebuffer->depthStencilBuffer.image,
			VK_IMAGE_VIEW_TYPE_2D,
			imageCI.format,
			componentMapping,
			imageSubresourceRange
		};
		vkCreateImageView(device, &imageViewCI, deviceAllocator, &(outFramebuffer->depthStencilView));
		assert(outFramebuffer->depthStencilView != VK_NULL_HANDLE);
	}

	std::vector<VkImageView> attachments_v(outFramebuffer->colorBuffers_v.size());
	for (size_t ca = 0; ca < outFramebuffer->colorBuffers_v.size(); ++ca)
	{
		attachments_v[ca] = outFramebuffer->colorViews_v[ca];
	}
	if (createDepthStencil)
	{
		attachments_v.push_back(outFramebuffer->depthStencilView);
	}

	VkFramebufferCreateInfo framebufferCI =
	{
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0,
		renderPass,
		(uint32_t)attachments_v.size(),
		attachments_v.data(),
		outFramebuffer->size.width,
		outFramebuffer->size.height,
		1
	};

	VkResult created = vkCreateFramebuffer(device, &framebufferCI, deviceAllocator, &(outFramebuffer->framebuffer));
	assert(created == VK_SUCCESS);
}

bhResourceID bhDeviceVK::CreateFramebuffer(bhSize2Di size, uint32_t numColorAttachments, bool createDepthStencil)
{
	bhFramebufferVK* outFramebuffer = new bhFramebufferVK();
	outFramebuffer->size = { uint32_t(size.width), uint32_t(size.height) };
	outFramebuffer->deleteColorBuffers = false;

	// Color
	{
		VkImageCreateInfo imageCI =
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_R8G8B8A8_UNORM,
			{ uint32_t(size.width), uint32_t(size.height) },
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

		outFramebuffer->colorBuffers_v.resize(numColorAttachments);
		outFramebuffer->colorViews_v.resize(numColorAttachments);

		for (uint32_t ca = 0; ca < numColorAttachments; ++ca)
		{
			bhTextureVK* tmpTexture = CreateMemoryBackedImage(&imageCI);
			assert(tmpTexture != nullptr);
			outFramebuffer->colorBuffers_v[ca] = *tmpTexture;
			delete tmpTexture;

			imageViewCI.image = outFramebuffer->colorBuffers_v[ca].image;
			vkCreateImageView(device, &imageViewCI, deviceAllocator, &(outFramebuffer->colorViews_v[ca]));
		}
	}
	CreateFramebuffer_Common(outFramebuffer, createDepthStencil);
	return static_cast<bhResourceID>(outFramebuffer);
}

bhResourceID bhDeviceVK::CreateFramebuffer(VkExtent2D _size, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil)
{
	bhFramebufferVK* outFramebuffer = new bhFramebufferVK();
	outFramebuffer->size = _size;
	outFramebuffer->deleteColorBuffers = false;

	bhTextureVK dt;
	dt.image = image;
	outFramebuffer->colorBuffers_v.push_back(dt);

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

	outFramebuffer->colorViews_v.resize(1);
	vkCreateImageView(device, &imageViewCI, deviceAllocator, &(outFramebuffer->colorViews_v[0]));
	CreateFramebuffer_Common(outFramebuffer, createDepthStencil);
	return static_cast<bhResourceID>(outFramebuffer);
}

void bhDeviceVK::DestroyFramebuffer(bhResourceID fbId)
{
	bhFramebufferVK* fb = static_cast<bhFramebufferVK*>(fbId);
	if (fb->depthStencilView)
	{
		vkDestroyImageView(device, fb->depthStencilView, deviceAllocator);
		fb->depthStencilView = VK_NULL_HANDLE;
		DestroyTexture(&(fb->depthStencilBuffer));
	}

	for (size_t cv = 0; cv < fb->colorViews_v.size(); ++cv)
	{
		vkDestroyImageView(device, fb->colorViews_v[cv], deviceAllocator);
		if (fb->deleteColorBuffers)
		{
			DestroyTexture(&(fb->colorBuffers_v[cv]));
		}
	}
	fb->colorViews_v.clear();

	vkDestroyFramebuffer(device, fb->framebuffer, deviceAllocator);
}

void bhDeviceVK::UseFramebuffer(bhResourceID fbId)
{}

void* bhDeviceVK::GetFramebufferColorAttachment(bhResourceID fbId, size_t attachmentIdx)
{
	bhFramebufferVK* fb = static_cast<bhFramebufferVK*>(fbId);
	assert(attachmentIdx < fb->colorBuffers_v.size());
	return reinterpret_cast<void*>(fb->colorBuffers_v[attachmentIdx].image); // TODO: Not sure whether this is correct..
}

bhSize2Di bhDeviceVK::GetFramebufferSize(bhResourceID fbId)
{
	bhFramebufferVK* fb = static_cast<bhFramebufferVK*>(fbId);
	return { int(fb->size.width), int(fb->size.height) };
}
