#include <SDL3/SDL_assert.h>
#include "VK/bhDeviceVk.hpp"
#include "VK/bhInstanceVk.hpp"
#include "VK/bhMeshVk.hpp"
#include "bhSystem.hpp"
#include "bhConfig.hpp"
#include "bhLog.h"
#include "Platform/bhPlatform.hpp"
#include "bhUtil.hpp"
#include "Mesh/bhMesh.hpp"
#include "bhImage.hpp"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

namespace bhVk
{
  static RenderDevice g_renderDev;
  RenderDevice& GetRenderDevice() { return g_renderDev; }

  ////////////////////////////////////////////////////////////////////////////////
  Buffer CreateBuffer(const Device& dev, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize)
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

    VmaAllocationCreateInfo aci = {};
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    aci.usage = VMA_MEMORY_USAGE_AUTO; // See VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE

    Buffer newBuffer;
    if (vmaCreateBuffer(dev.allocator, &bufferCreateInfo, &aci, &(newBuffer.buffer), &(newBuffer.alloc), nullptr) == VK_SUCCESS)
    {
      // TODO: Report error
      return newBuffer;
    }

    DestroyBuffer(dev, newBuffer);
    return newBuffer;
  }

  void DestroyBuffer(
    const Device& dev,
    Buffer& buffer)
  {
    vmaDestroyBuffer(dev.allocator, buffer.buffer, buffer.alloc);
    buffer = {};
  }

  VkBool32 CopyDataToBuffer(const Device& dev, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data)
  {
    //SDL_assert(buffer.alloc != VK_NULL_HANDLE);
    if (vmaCopyMemoryToAllocation(dev.allocator, data, buffer.alloc, offset, reqSize) == VK_SUCCESS)
    {
      return VK_TRUE;
    }
    SDL_assert(false);
    return VK_FALSE;
  }

  VkCommandBuffer BeginSingleTimeCommandBuffer(const Device& dev)
  {
    VkCommandBufferAllocateInfo commandBufferAI =
    {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
      dev.commandPool,
      VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      1
    };

    VkCommandBuffer outCommandBuffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(dev.device, &commandBufferAI, &outCommandBuffer) != VK_SUCCESS)
    {
      return VK_NULL_HANDLE;
    }
    VkCommandBufferBeginInfo commandBufferBI =
    {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    return (vkBeginCommandBuffer(outCommandBuffer, &commandBufferBI) == VK_SUCCESS) ? outCommandBuffer : VK_NULL_HANDLE;
  }

  void EndSingleTimeCommandBuffer(const Device& dev, VkCommandBuffer commandBuffer, VkQueue queue)
  {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo =
    {
      VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr,
      0,
      nullptr,
      nullptr,
      1,
      &commandBuffer
    };

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(dev.device, dev.commandPool, 1, &commandBuffer);
  }

  bool CreateTexture(const Device& dev, Texture& newTex, const VkImageCreateInfo& imageCI)
  {
    VmaAllocationCreateInfo aci = {};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    if (vmaCreateImage(dev.allocator, &imageCI, &aci, &(newTex.image), &(newTex.alloc), nullptr) == VK_SUCCESS)
    {
      newTex.extent = imageCI.extent;
      newTex.format = imageCI.format;
      newTex.mipLevels = 1;
      return true;
    }
    return false;
  }

  bool CreateTexture(const Device& dev, Texture& newTex, VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels)
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

    return CreateTexture(dev, newTex, imageCI);
  }

  Buffer CreateBufferFromImage(const Device& dev, const bhImage& img)
  {
    VkDeviceSize imageSize = bhImage::GetMemSiz(img);

    VkBufferCreateInfo bufferCreateInfo =
    {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0,
      imageSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      0,
      nullptr
    };

    VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    Buffer newBuffer = CreateBuffer(dev, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, propertyFlags, imageSize);

    if (CopyDataToBuffer(dev, newBuffer, 0, imageSize, img.pixels) != VK_TRUE)
    {
      DestroyBuffer(dev, newBuffer);
    }
    return newBuffer;
  }

  bool CreateTextureFromImage(const Device& dev, Texture& newTex, const bhImage& img, int32_t offsetx, int32_t offsety)
  {
    // Try to make sense out of img components
    VkFormat fmt = PickImageFormat(img);
    if (fmt == VK_FORMAT_UNDEFINED)
    {
      return {};
    }

    VkExtent3D extent = {};
    extent.width = img.width;
    extent.height = img.height;
    extent.depth = img.depth;

    // Create the empty texture with memory
    uint32_t requiredMipLevels = bhImage::GetRequiredMipLevels(img);
    if (!CreateTexture(dev, newTex, MapImageTypeToVk(bhImage::DetermineDimensions(img)), fmt, extent, requiredMipLevels))
    {
      return false;
    }
    //if (!IsValid(newTex))
    //{
    //  return {};
    //}

    // Create the buffer from pixel data to copy the texture img from
    Buffer imageBuffer = CreateBufferFromImage(dev, img);
    if (!IsValid(imageBuffer))
    {
      return {};
    }

    VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(dev);
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

      const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
      if (rs.GenMipmaps())
      {
        Texture::GenerateMipmaps(newTex, commandBuffer, requiredMipLevels);
      }

      ////////////////////////////////////////////////////////////////////////////////
      // Make img shader readable
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      imageMemoryBarrier.oldLayout = rs.GenMipmaps() ?
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL :
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

      EndSingleTimeCommandBuffer(dev, commandBuffer, dev.queue);
    }
    // Clear intermediate img source buffer
    DestroyBuffer(dev, imageBuffer);
    return true;
  }

  void DestroyTexture(const Device& dev, Texture& texture, bool destroyColorImage)
  {
    vmaDestroyImage(dev.allocator, texture.image, texture.alloc);
    texture = {};
  }

  ////////////////////////////////////////////////////////////////////////////////
  uint32_t RankPhysicalDeviceGraphicsFeatures(const VkPhysicalDevice physicalDevice)
  {
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

    const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
    uint32_t result = 0;

    VkBool32 reqAnisotropy = (rs.anisotropy_level > 0) ? VK_TRUE : VK_FALSE;
    if (physicalDeviceFeatures.samplerAnisotropy == reqAnisotropy)
    {
      ++result;
    }
    return result;
  }

  struct SelectedDevice
  {
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    uint32_t queueFamilyIdx{ UINT32_MAX };
  };

  VkBool32 IsValid(const SelectedDevice& sd) { return (sd.physicalDevice != VK_NULL_HANDLE) && (sd.queueFamilyIdx < UINT32_MAX) ? VK_TRUE : VK_FALSE; }

  SelectedDevice ChoosePhysicalRenderDevice(const VkInstance inst, const VkSurfaceKHR wndSurf,
    const VkQueueFamilyProperties& reqQFP_Graphics, const std::vector<const char*>& reqExtNames_v)
  {
    uint32_t numPhysicalDevices = 0;
    if (vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, nullptr) != VK_SUCCESS)
    {
      // TODO: Log Error?
      return {};
    }
    std::vector<VkPhysicalDevice> physicalDevices_v(numPhysicalDevices);
    if (vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, physicalDevices_v.data()) != VK_SUCCESS)
    {
      // TODO: Log Error?
      return {};
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

      queueFamilyInds_v[i] = GetQueueFamily(physicalDevices_v[i], reqQFP_Graphics, wndSurf);
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

    SelectedDevice sd = { physicalDevices_v[maxRankIdx] ,queueFamilyInds_v[maxRankIdx] };
    if (IsValid(sd))
    {
      LogPhysicalDevice(sd.physicalDevice);
      return sd;
    }
    return {};
  }

  VkBool32 CreateDescriptorPool(RenderDevice& dev)
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
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      maxSets,
      (uint32_t)descriptorPoolSizes_v.size(),
      descriptorPoolSizes_v.data()
    };

    VkResult result = vkCreateDescriptorPool(dev.device, &descriptorPoolCI, dev.alloc, &(dev.descriptorPool));
    return (result == VK_SUCCESS) ? VK_TRUE : VK_FALSE;
  }

  VkBool32 CreatePresentRenderPass(RenderDevice& dev, VkSurfaceKHR wndSurf)
  {
    std::vector<VkAttachmentDescription> attachmentDescriptions_v(2);
    {
      attachmentDescriptions_v[0].flags = 0;
      attachmentDescriptions_v[0].format = DetermineSurfaceFormat(dev.physicalDevice, wndSurf).format;
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

    if (vkCreateRenderPass(dev.device, &renderPassCI, dev.alloc, &(dev.presentRenderPass)) == VK_SUCCESS)
    {
      dev.clearValues_v[0].color.float32[0] = 1.0f;
      dev.clearValues_v[0].color.float32[1] = 0.0f;
      dev.clearValues_v[0].color.float32[2] = 1.0f;
      dev.clearValues_v[0].color.float32[3] = 1.0f;

      dev.clearValues_v[1].depthStencil.depth = 1.0f;
      dev.clearValues_v[1].depthStencil.stencil = 0;

      return VK_TRUE;
    }
    return VK_FALSE;
  }

  VkBool32 CreateSwapchain(RenderDevice& dev, VkSurfaceKHR presentSurf, uint32_t numImages)
  {
    // Returns the number of swap chain images, so 0 meeans failure
    VkSurfaceFormatKHR surfaceFormat = DetermineSurfaceFormat(dev.physicalDevice, presentSurf);
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
      return VK_FALSE;
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!IsPresentModeSupported(dev.physicalDevice, presentSurf, presentMode))
    {
      return VK_FALSE;
    }

    VkExtent2D wndExtent = {};
    const bhConfig::WindowSettings& ws = bhSystem::Config()->windowSt;
    wndExtent.width = ws.w;
    wndExtent.height = ws.h;

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

    if (!TestPhysicalDeviceSurfaceCaps(dev.physicalDevice, presentSurf, &surfaceCapabilities))
    {
      return VK_FALSE;
    }

    VkSwapchainCreateInfoKHR swapChainCI =
    {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr , 0,
      presentSurf, //surface
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
    if (vkCreateSwapchainKHR(dev.device, &swapChainCI, dev.alloc, &(dev.swapChain.swapChain)) == VK_SUCCESS)
    {
      if (vkGetSwapchainImagesKHR(dev.device, dev.swapChain.swapChain, &numSwapChainImages, nullptr) == VK_SUCCESS)
      {
        std::vector<VkImage> swapChainImages_v(numSwapChainImages);
        if (vkGetSwapchainImagesKHR(dev.device, dev.swapChain.swapChain, &numSwapChainImages, swapChainImages_v.data()) == VK_SUCCESS)
        {
          //Setup Render targets
          dev.swapChain.frameBuffers_v.resize(numSwapChainImages);
          for (uint32_t fbIdx = 0; fbIdx < numSwapChainImages; ++fbIdx)
          {
            CreateFramebuffer(dev.swapChain.frameBuffers_v[fbIdx], dev, wndExtent, swapChainImages_v.data()[fbIdx], surfaceFormat, true);
          }
        }
      }
    }
    return (numSwapChainImages > 0) ? VK_TRUE : VK_FALSE;
  }

  VkBool32 CreateCommandBuffers(RenderDevice& dev, uint32_t numBuffers)
  {
    VkCommandBufferAllocateInfo commandBufferAI =
    {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
      dev.commandPool,
      VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      numBuffers
    };

    dev.commandBuffers_v.resize(numBuffers);
    VkResult res = vkAllocateCommandBuffers(dev.device, &commandBufferAI, dev.commandBuffers_v.data());
    return res == VK_SUCCESS ? VK_TRUE : VK_FALSE;
  }

  VmaAllocator CreateAllocator(VkPhysicalDevice physDevice, VkDevice device, const Instance& inst)
  {
    VmaVulkanFunctions vf = {};
    vf.vkAllocateMemory = vkAllocateMemory;
    vf.vkBindBufferMemory = vkBindBufferMemory;
    vf.vkBindBufferMemory2KHR = vkBindBufferMemory2;
    vf.vkBindImageMemory = vkBindImageMemory;
    vf.vkBindImageMemory2KHR = vkBindImageMemory2;
    vf.vkCmdCopyBuffer = vkCmdCopyBuffer;
    vf.vkCreateBuffer = vkCreateBuffer;
    vf.vkCreateImage = vkCreateImage;
    vf.vkDestroyBuffer = vkDestroyBuffer;
    vf.vkDestroyImage = vkDestroyImage;
    vf.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vf.vkFreeMemory = vkFreeMemory;
    vf.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vf.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
    vf.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    vf.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
    vf.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vf.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vf.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
    vf.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vf.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vf.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    vf.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vf.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vf.vkMapMemory = vkMapMemory;
    vf.vkUnmapMemory = vkUnmapMemory;

    VmaAllocatorCreateInfo aci = {};
    //aci.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    aci.vulkanApiVersion = VK_API_VERSION_1_1; //Sync wih bhConfig::RenderSettings::VKSettings
    aci.physicalDevice = physDevice;
    aci.device = device;
    aci.instance = inst.instance;
    aci.pVulkanFunctions = &vf;

    VmaAllocator newAlloc;
    BH_ASSERT_VK(vmaCreateAllocator(&aci, &newAlloc));
    return newAlloc;
  }

  VkBool32 CreateRenderDevice(RenderDevice& dev, const Instance& inst, const VkSurfaceKHR wndSurf)
  {
    VkQueueFamilyProperties reqQFP_Graphics = {};
    reqQFP_Graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT;
    reqQFP_Graphics.queueCount = 1;

    std::vector<const char*> reqExtNames_v = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
    SelectedDevice sd = ChoosePhysicalRenderDevice(inst.instance, wndSurf, reqQFP_Graphics, reqExtNames_v);
    if (!IsValid(sd))
    {
      return VK_FALSE;
    }
    dev.physicalDevice = sd.physicalDevice;
    dev.queueFamilyIdx = sd.queueFamilyIdx;

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    vkGetPhysicalDeviceFeatures(sd.physicalDevice, &physicalDeviceFeatures);
    //TODO - we should provide only the required features

    const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
    physicalDeviceFeatures.samplerAnisotropy = (rs.anisotropy_level > 0);

    std::vector<float> queuePriorities_v(reqQFP_Graphics.queueCount, 1.0f);

    VkDeviceQueueCreateInfo deviceQueueCI =
    {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
      dev.queueFamilyIdx,
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

    if (vkCreateDevice(dev.physicalDevice, &deviceCI, inst.alloc, &(dev.device)) != VK_SUCCESS) return VK_FALSE;
    dev.allocator = CreateAllocator(dev.physicalDevice, dev.device, inst);

    VkCommandPoolCreateInfo commandPoolCreateInfo =
    {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      dev.queueFamilyIdx
    };
    if (vkCreateCommandPool(dev.device, &commandPoolCreateInfo, dev.alloc, &(dev.commandPool)) != VK_SUCCESS) return VK_FALSE;

    if (!CreateDescriptorPool(dev)) return VK_FALSE;
    if (!CreatePresentRenderPass(dev, wndSurf)) return VK_FALSE;

    if (!CreateSwapchain(dev, wndSurf, rs.num_swapchain_images)) return VK_FALSE;

    SDL_assert(rs.num_swapchain_images == dev.swapChain.frameBuffers_v.size());
    CreateCommandBuffers(dev, rs.num_swapchain_images);
    vkGetDeviceQueue(dev.device, dev.queueFamilyIdx, 0, &(dev.queue));

    // Create sync primitives
    VkFenceCreateInfo fenceCI = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    BH_ASSERT_VK(vkCreateFence(dev.device, &fenceCI, dev.alloc, &(dev.drawFence)));

    VkSemaphoreCreateInfo semaphoreCI = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    BH_ASSERT_VK(vkCreateSemaphore(dev.device, &semaphoreCI, dev.alloc, &(dev.sph_ImageAvailable)));
    BH_ASSERT_VK(vkCreateSemaphore(dev.device, &semaphoreCI, dev.alloc, &(dev.sph_RenderFinished)));

    //g_clearRects_v[0] = {};
    //g_clearRects_v[0].rect.extent.width = bhConfig::GetWidth();
    //g_clearRects_v[0].rect.extent.height = bhConfig::GetHeight();

    //s_textureCache.Init();

    VkMemoryPropertyFlags propFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    dev.vertexBuffer = CreateBuffer(dev, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, propFlags, 1024 * 1024);
    dev.indexBuffer = CreateBuffer(dev, VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT, propFlags, 1024 * 1024);

    return VK_TRUE;
  }

  void DestroySwapchain(RenderDevice& dev)
  {
    for (size_t fb = 0; fb < dev.swapChain.frameBuffers_v.size(); ++fb)
    {
      DestroyFramebuffer(dev.swapChain.frameBuffers_v[fb], dev);
    }
    dev.swapChain.frameBuffers_v.clear();
    dev.swapChain.swapChainIdx = UINT32_MAX;

    vkDestroySwapchainKHR(dev.device, dev.swapChain.swapChain, dev.alloc);
    dev.swapChain.swapChain = VK_NULL_HANDLE;
  }

  void DestroyRenderDevice(RenderDevice& dev, const Instance& inst)
  {
    vkDeviceWaitIdle(dev.device);

    //for (auto& tx : s_textureCache.GetTextures())
    //{
    //  DestroyTexture(tx.second);
    //}

    //textureManager.DestroyAll([this](bhTextureVK* t)
    //	{
    //		this->DestroyTexture(t);
    //	});

    DestroyBuffer(dev, dev.indexBuffer);
    DestroyBuffer(dev, dev.vertexBuffer);

    // Destroy sync primitives
    vkDestroySemaphore(dev.device, dev.sph_RenderFinished, dev.alloc);
    vkDestroySemaphore(dev.device, dev.sph_ImageAvailable, dev.alloc);

    vkDestroyFence(dev.device, dev.drawFence, dev.alloc);

    vkFreeCommandBuffers(dev.device, dev.commandPool, static_cast<uint32_t>(dev.commandBuffers_v.size()), dev.commandBuffers_v.data());
    dev.commandBuffers_v.clear();

    DestroySwapchain(dev);
    vkDestroyRenderPass(dev.device, dev.presentRenderPass, dev.alloc);

    //vkDeviceWaitIdle(dev.device);

    vkDestroyDescriptorPool(dev.device, dev.descriptorPool, dev.alloc);
    vkDestroyCommandPool(dev.device, dev.commandPool, dev.alloc);
    vmaDestroyAllocator(dev.allocator);
    vkDestroyDevice(dev.device, dev.alloc);
  }

  void BeginFrame(RenderDevice& dev)
  {
    vkAcquireNextImageKHR(dev.device, dev.swapChain.swapChain, UINT64_MAX, dev.sph_ImageAvailable, VK_NULL_HANDLE, &(dev.swapChain.swapChainIdx));

    VkCommandBufferBeginInfo commandBufferBI =
    {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(dev.commandBuffers_v[dev.swapChain.swapChainIdx], &commandBufferBI);
    //vkCmdClearAttachments(commandBuffers_v[swapChainIdx], config_ClearAttachmentCount, config_ClearAttachments, g_numClearRects, g_clearRects_v);

    // Begin render pass
    VkRect2D renderArea =
    {
      {0, 0},
      dev.swapChain.frameBuffers_v[dev.swapChain.swapChainIdx].extent
    };

    VkRenderPassBeginInfo renderPassBI =
    {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr,
      dev.presentRenderPass,
      dev.swapChain.frameBuffers_v[dev.swapChain.swapChainIdx].framebuffer,
      renderArea,
      2, dev.clearValues_v
    };

    vkCmdBeginRenderPass(dev.commandBuffers_v[dev.swapChain.swapChainIdx], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
  }

  void EndFrame(RenderDevice& dev)
  {
    vkCmdEndRenderPass(dev.commandBuffers_v[dev.swapChain.swapChainIdx]);

    vkEndCommandBuffer(dev.commandBuffers_v[dev.swapChain.swapChainIdx]);

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo =
    {
      VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr,
      1,
      &(dev.sph_ImageAvailable),
      waitStages,
      1,
      &(dev.commandBuffers_v[dev.swapChain.swapChainIdx]),
      1,
      &(dev.sph_RenderFinished)
    };
    vkQueueSubmit(dev.queue, 1, &submitInfo, dev.drawFence);

    vkWaitForFences(dev.device, 1, &(dev.drawFence), VK_TRUE, UINT64_MAX);
    vkResetFences(dev.device, 1, &(dev.drawFence));

    VkPresentInfoKHR presentInfo =
    {
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr,
      1,
      &(dev.sph_RenderFinished),
      1,
      &(dev.swapChain.swapChain),
      &(dev.swapChain.swapChainIdx)
    };
    vkQueuePresentKHR(dev.queue, &presentInfo);
    ++dev.frameCount;
  }

  VkShaderModule CreateShaderFromFile(const RenderDevice& dev, const char* fileName)
  {
    VkShaderModule newShaderMd = VK_NULL_HANDLE;

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

      VkResult result = vkCreateShaderModule(dev.device, &shaderModuleCI, dev.alloc, &newShaderMd);
      bhUtil::FreeFileData(fileData);
    }
    bhPlatform::FreePath(path);
    SDL_assert(newShaderMd != VK_NULL_HANDLE);
    return newShaderMd;
  }

  VkImageView CreateTextureImageView(const RenderDevice& dev, const Texture& tx)
  {
    if (tx.format == VK_FORMAT_UNDEFINED)
    {
      return VK_NULL_HANDLE;
    }

    VkImageSubresourceRange imageSubresourceRange =
    {
      VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
      0, //baseMipLevel
      tx.mipLevels, //levelCount
      0, //baseArrayLayer
      1 //layerCount
    };

    VkComponentMapping componentMapping = {};
    VkImageViewCreateInfo imageViewCI =
    {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
      tx.image,
      VK_IMAGE_VIEW_TYPE_2D,
      tx.format,
      componentMapping,
      imageSubresourceRange
    };

    VkImageView imageView = VK_NULL_HANDLE;
    vkCreateImageView(dev.device, &imageViewCI, dev.alloc, &imageView);
    return imageView;
  }

  float GetAnisotropy(const RenderDevice& dev)
  {
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    vkGetPhysicalDeviceFeatures(dev.physicalDevice, &physicalDeviceFeatures);
    if (physicalDeviceFeatures.samplerAnisotropy)
    {
      const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
      return (rs.anisotropy_level == 0) ? 0.0f : float(1 << rs.anisotropy_level);
    }
    return 0.0f;
  }

  VkDescriptorImageInfo SetupTextureDescriptor(const RenderDevice& dev, const Texture& tx)
  {
    VkDescriptorImageInfo descriptorImgInfo = {};
    descriptorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImgInfo.imageView = CreateTextureImageView(dev, tx);

    float reqAnisotropy = GetAnisotropy(dev);
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

    VkResult res = vkCreateSampler(dev.device, &samplerCI, dev.alloc, &descriptorImgInfo.sampler);
    if (res != VK_SUCCESS)
    {
      // Report VK_ERROR_OUT_OF_HOST_MEMORY / VK_ERROR_OUT_OF_DEVICE_MEMORY
    }
    return descriptorImgInfo;
  }

  bool CreateImGui(const RenderDevice& dev, const Instance& inst, SDL_Window* wnd)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    if (ImGui_ImplSDL3_InitForVulkan(wnd))
    {
      const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
      static const size_t VERSION_SRING_LEN = 16;
      char versionString[VERSION_SRING_LEN];
      sprintf_s(versionString, VERSION_SRING_LEN, "#version %d%d0", rs.vk.versionMajor, rs.vk.versionMinor);

      ImGui_ImplVulkan_InitInfo initInfo = {};
      initInfo.Instance = inst.instance;
      initInfo.PhysicalDevice = dev.physicalDevice;
      initInfo.Device = dev.device;
      initInfo.QueueFamily = dev.queueFamilyIdx;
      initInfo.Queue = dev.queue;
      initInfo.DescriptorPool = dev.descriptorPool;
      initInfo.RenderPass = dev.presentRenderPass;
      initInfo.MinImageCount = 2;
      initInfo.ImageCount = static_cast<uint32_t>(dev.swapChain.frameBuffers_v.size());
      initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
      //initInfo.PipelineCache = ;
      //initInfo.Subpass = ;
      initInfo.Allocator = dev.alloc;
      initInfo.CheckVkResultFn = CheckVkResult;

      ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
        return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
        }, inst.instance);
      if (ImGui_ImplVulkan_Init(&initInfo))
        // Upload Fonts
      {
        //  // Use any command queue
        //  VkCommandBuffer command_buffer = dev.commandBuffers_v[0];
        //  BH_ASSERT_VK(vkResetCommandPool(dev.device, dev.commandPool, 0));

        //  VkCommandBufferBeginInfo beginInfo = {};
        //  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //  beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        //  BH_ASSERT_VK(vkBeginCommandBuffer(command_buffer, &beginInfo));
        //ImGui_ImplVulkan_CreateFontsTexture();

        //  VkSubmitInfo end_info = {};
        //  end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //  end_info.commandBufferCount = 1;
        //  end_info.pCommandBuffers = &command_buffer;
        //  BH_ASSERT_VK(vkEndCommandBuffer(command_buffer));
        //  BH_ASSERT_VK(vkQueueSubmit(dev.queue, 1, &end_info, VK_NULL_HANDLE));

        //  BH_ASSERT_VK(vkDeviceWaitIdle(dev.device));
        return true;
      }
    }
    return false;
  }

  void DestroyImGui()
  {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }

  void BeginImGuiFrame(RenderDevice& dev)
  {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
  }

  void EndImGuiFrame(RenderDevice& dev)
  {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), dev.commandBuffers_v[dev.swapChain.swapChainIdx]);
  }

  VkBool32 CreateDeviceMesh(Mesh& devMesh, const RenderDevice& dev, const bhMesh& mesh)
  {
    VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;// | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkDeviceSize reqVertexSize = mesh.GetNumVerts() * sizeof(bhMeshVertex_t);
    devMesh.vb = CreateBuffer(dev, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, propertyFlags, reqVertexSize);
    VkBool32 vbSuccess = IsValid(devMesh.vb) & CopyDataToBuffer(dev, devMesh.vb, 0, reqVertexSize, mesh.GetVerts());
    if (!vbSuccess)
    {
      DestroyBuffer(dev, devMesh.vb);
    }

    VkDeviceSize reqIdxSize = mesh.GetNumInds() * sizeof(bhMeshIdx_t);
    devMesh.ib = CreateBuffer(dev, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, propertyFlags, reqIdxSize);
    VkBool32 ibSuccess = IsValid(devMesh.ib) & CopyDataToBuffer(dev, devMesh.ib, 0, reqIdxSize, mesh.GetInds());
    if (!ibSuccess)
    {
      DestroyBuffer(dev, devMesh.ib);
    }
    devMesh.indexCount = mesh.GetNumInds();
    return vbSuccess & ibSuccess;
  }

  void DestroyDeviceMesh(const RenderDevice& dev, Mesh& devMesh)
  {
    DestroyBuffer(dev, devMesh.ib);
    DestroyBuffer(dev, devMesh.vb);
  }

  void BindDeviceMesh(const RenderDevice& dev, const bhVk::Mesh& mesh)
  {
    VkCommandBuffer currCB = dev.commandBuffers_v[dev.swapChain.swapChainIdx];
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(currCB, 0, 1, &(mesh.vb.buffer), offsets);

  #if BH_USE_MESH_INDEX_TYPE_UNIT16
    vkCmdBindIndexBuffer(currCB, mesh.ib.buffer, 0, VK_INDEX_TYPE_UINT16);
  #else
    vkCmdBindIndexBuffer(currCB, mesh.ib.buffer, 0, VK_INDEX_TYPE_UINT32);
  #endif
  }

  void DrawDeviceMesh(const RenderDevice& dev, VkPipelineLayout plLayout, const bhVk::Mesh& mesh, const glm::mat4& transform)
  {
    VkCommandBuffer currCB = dev.commandBuffers_v[dev.swapChain.swapChainIdx];
    vkCmdPushConstants(currCB, plLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
    vkCmdDrawIndexed(currCB, mesh.indexCount, 1, 0, 0, 0);
  }
}
