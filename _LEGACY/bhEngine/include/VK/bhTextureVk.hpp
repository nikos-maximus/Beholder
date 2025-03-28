#ifndef BH_TEXTURE_VK_HPP
#define BH_TEXTURE_VK_HPP

#include <string>
#include "VK/bhIncludeVk.hpp"
#include "bhHash.hpp"

namespace bhVk
{
  struct Texture
  {
    VkImage image{ VK_NULL_HANDLE };
    VmaAllocation alloc{ VK_NULL_HANDLE };
    VkFormat format{ VK_FORMAT_UNDEFINED };
    VkExtent3D extent{};
    uint32_t mipLevels{ 0 };

    static void GenerateMipmaps(Texture& texture, VkCommandBuffer commandBuffer, uint32_t numMipLevels);
  };

  inline bool IsValid(const Texture& tx) { return (tx.image != VK_NULL_HANDLE); }
}

#endif //BH_TEXTURE_VK_HPP
