#include <assert.h>
#include "VK/bhVKFramebuffer.hpp"
#include "VK/bhVKRenderDevice.hpp"

VkBool32 bhVKFramebuffer::Create(const bhVKRenderDevice* rd, VkExtent2D insize, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil)
{
  size = insize;
  deleteColorBuffers = false;

  bhVKTexture dt;
  dt.SetImage(image);
  colorTextures_v.push_back(dt);

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

  colorViews_v.resize(1);
  vkCreateImageView(rd->Device(), &imageViewCI, rd->AllocCB(), &(colorViews_v[0]));

  if (createDepthStencil)
  {
    CreateDepthStencilAttachment(rd);
  }

  return Instantiate(rd);
}

VkBool32 bhVKFramebuffer::CreateColorAttachments(const bhVKRenderDevice* rd, uint32_t numColorAttachments)
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

  colorTextures_v.resize(numColorAttachments);
  colorViews_v.resize(numColorAttachments);

  for (uint32_t ca = 0; ca < numColorAttachments; ++ca)
  {
    colorTextures_v[ca] = std::move(rd->CreateTexture(imageCI));
    assert(colorTextures_v[ca].IsValid());
    imageViewCI.image = colorTextures_v[ca].Image();
    if (vkCreateImageView(rd->Device(), &imageViewCI, rd->AllocCB(), &(colorViews_v[ca])) != VK_SUCCESS)
    {
      return VK_FALSE;
    }
  }
  return VK_TRUE;
}

VkBool32 bhVKFramebuffer::CreateDepthStencilAttachment(const bhVKRenderDevice* rd)
{
  VkImageCreateInfo imageCI =
  {
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
    VK_IMAGE_TYPE_2D,
    VK_FORMAT_D24_UNORM_S8_UINT,
    { size.width, size.height, 1 },
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
    depthStencilTexture = std::move(rd->CreateTexture(imageCI));
    assert(depthStencilTexture.IsValid());
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
    depthStencilTexture.Image(),
    VK_IMAGE_VIEW_TYPE_2D,
    imageCI.format,
    componentMapping,
    imageSubresourceRange
  };
  if (vkCreateImageView(rd->Device(), &imageViewCI, rd->AllocCB(), &(depthStencilView)) != VK_SUCCESS)
  {
    return VK_FALSE;
  }
  return VK_TRUE;
}

VkBool32 bhVKFramebuffer::Instantiate(const bhVKRenderDevice* rd)
{
  std::vector<VkImageView> attachments_v;
  for (auto& cv : colorViews_v)
  {
    attachments_v.push_back(cv);
  }
  if (depthStencilView != VK_NULL_HANDLE)
  {
    attachments_v.push_back(depthStencilView);
  }

  VkFramebufferCreateInfo framebufferCI =
  {
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0,
    rd->PresentRenderPass(),
    (uint32_t)attachments_v.size(),
    attachments_v.data(),
    size.width,
    size.height,
    1
  };

  if (vkCreateFramebuffer(rd->Device(), &framebufferCI, rd->AllocCB(), &(framebuffer)) == VK_SUCCESS)
  {
    return VK_TRUE;
  }
  return VK_FALSE;
}

void bhVKFramebuffer::Destroy(const bhVKRenderDevice* rd)
{
  if (depthStencilView)
  {
    vkDestroyImageView(rd->Device(), depthStencilView, rd->AllocCB());
    depthStencilView = VK_NULL_HANDLE;
    rd->DestroyTexture(depthStencilTexture);
  }

  for (size_t cv = 0; cv < colorViews_v.size(); ++cv)
  {
    vkDestroyImageView(rd->Device(), colorViews_v[cv], rd->AllocCB());
    if (deleteColorBuffers)
    {
      rd->DestroyTexture(colorTextures_v[cv]);
    }
  }
  colorViews_v.clear();

  vkDestroyFramebuffer(rd->Device(), framebuffer, rd->AllocCB());
}
