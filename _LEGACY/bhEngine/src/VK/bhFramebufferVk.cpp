#include <SDL3/SDL_assert.h>
#include "VK/bhFramebufferVk.hpp"
#include "VK/bhDeviceVk.hpp"

namespace bhVk
{
  VkBool32 CreateFramebufferDepthStencilAttachment(Framebuffer& fb, const RenderDevice& rd)
  {
    VkImageCreateInfo imageCI =
    {
      VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
      VK_IMAGE_TYPE_2D,
      VK_FORMAT_D24_UNORM_S8_UINT,
      { fb.extent.width, fb.extent.height, 1 },
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
      CreateTexture(rd, fb.depthStencilTexture, imageCI); //TODO: Error check
      SDL_assert(IsValid(fb.depthStencilTexture));
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
      fb.depthStencilTexture.image,
      VK_IMAGE_VIEW_TYPE_2D,
      imageCI.format,
      componentMapping,
      imageSubresourceRange
    };

    return vkCreateImageView(rd.device, &imageViewCI, rd.alloc, &(fb.depthStencilView)) == VK_SUCCESS ? VK_TRUE : VK_FALSE;
  }

  VkBool32 CreateFramebuffer(Framebuffer& fb, const RenderDevice& rd, VkExtent2D insize, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil)
  {
    fb.extent = insize;
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

    vkCreateImageView(rd.device, &imageViewCI, rd.alloc, &(fb.colorView));

    if (createDepthStencil)
    {
      CreateFramebufferDepthStencilAttachment(fb, rd);
    }

    return Instantiate(fb, rd);
  }

  VkBool32 Instantiate(Framebuffer& fb, const RenderDevice& rd)
  {
    SDL_assert(fb.colorView != VK_NULL_HANDLE);

    std::vector<VkImageView> attachments_v;
    attachments_v.push_back(fb.colorView);
    if (fb.depthStencilView != VK_NULL_HANDLE)
    {
      attachments_v.push_back(fb.depthStencilView);
    }

    VkFramebufferCreateInfo framebufferCI =
    {
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0,
      rd.presentRenderPass,
      (uint32_t)attachments_v.size(),
      attachments_v.data(),
      fb.extent.width,
      fb.extent.height,
      1
    };

    if (vkCreateFramebuffer(rd.device, &framebufferCI, rd.alloc, &(fb.framebuffer)) == VK_SUCCESS)
    {
      return VK_TRUE;
    }
    return VK_FALSE;
  }

  void DestroyFramebuffer(Framebuffer& fb, const RenderDevice& rd)
  {
    if (fb.depthStencilView)
    {
      DestroyTexture(rd, fb.depthStencilTexture);
      vkDestroyImageView(rd.device, fb.depthStencilView, rd.alloc);
      fb.depthStencilView = VK_NULL_HANDLE;
    }

    vkDestroyImageView(rd.device, fb.colorView, rd.alloc);
    vkDestroyFramebuffer(rd.device, fb.framebuffer, rd.alloc);
  }
}
