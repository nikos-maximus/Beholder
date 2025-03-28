#include "bhContext.h"
#include "bhConfig.h"
#include "bhImage.h"
#include "bhLog.h"

////////////////////////////////////////////////////////////////////////////////
VkRenderPass bhContext::CreateRenderPass(const bhdevicevk)
{
	std::vector<VkAttachmentDescription> attachmentDescriptions_v(2);
	{
		attachmentDescriptions_v[0].flags = 0;
		attachmentDescriptions_v[0].format = windowSurfaceFormat.format;
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

	VkRenderPassCreateInfo renderPassCI = VkRenderPassCreateInfo_Init();
	renderPassCI.attachmentCount = attachmentDescriptions_v.size();
	renderPassCI.pAttachments = attachmentDescriptions_v.data();
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpassDescription;
	renderPassCI.dependencyCount = 1;
	renderPassCI.pDependencies = &subpassDependency;

	VkRenderPass newRenderPass = VK_NULL_HANDLE;
	if (vkCreateRenderPass(device, &renderPassCI, allocator, &newRenderPass) == VK_SUCCESS)
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
