#include "VK/bhVKTexture.hpp"

bhVKTexture::bhVKTexture(const Properties& p)
{
  SetProperties(p);
}

void bhVKTexture::SetProperties(const Properties& props)
{
  image = props.image;
  memory = props.memory;
  fmt = props.fmt;
  mipLevels = props.mipLevels;

  extent.width = props.ext.width;
  extent.height = props.ext.height;
  extent.depth = props.ext.depth;
}

void bhVKTexture::GenerateMipmaps(VkCommandBuffer commandBuffer, uint32_t numMipLevels)
{
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
    image, //image
    imageSubresourceRange //subresourceRange
  };
  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

  uint32_t w = extent.width, h = extent.height;

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
    vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

    // Make mip level readable for next blit iteration
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
  }
  //imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  //imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  //vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

  mipLevels = numMipLevels;
}
