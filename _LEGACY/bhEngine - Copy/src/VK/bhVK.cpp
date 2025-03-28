#include <assert.h>
#include "VK/bhVKDevice.hpp"
#include "bhLog.h"
#include "Texture/bhImage.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bhVK
{
  VkBool32 IsExtentValid2D(const VkExtent2D& physicalDeviceExtent, const VkExtent2D& reqExtent)
  {
    return (physicalDeviceExtent.width >= reqExtent.width) && (physicalDeviceExtent.height >= reqExtent.height) ? VK_TRUE : VK_FALSE;
  }

  VkBool32 IsExtentValid3D(const VkExtent3D& physicalDeviceExtent, const VkExtent3D& reqExtent)
  {
    return (physicalDeviceExtent.width >= reqExtent.width) && (physicalDeviceExtent.height >= reqExtent.height) && (physicalDeviceExtent.depth >= reqExtent.depth) ? VK_TRUE : VK_FALSE;
  }

  VkShaderStageFlagBits GetShaderStage(bhShaderStage shaderType)
  {
    VkShaderStageFlagBits types[NUM_SHADER_STAGES] =
    {
      VK_SHADER_STAGE_VERTEX_BIT,
      VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
      VK_SHADER_STAGE_GEOMETRY_BIT,
      VK_SHADER_STAGE_FRAGMENT_BIT,
      VK_SHADER_STAGE_COMPUTE_BIT
    };
    return types[(int)shaderType];
  }

  const char* GetPhysicalDeviceTypeString(VkPhysicalDeviceType physicalDeviceType)
  {
    const char* names[] =
    {
      "Other",
      "Integrated GPU",
      "Discrete GPU",
      "Virtual GPU",
      "CPU",
    };
    return names[physicalDeviceType];
  }

  void LogPhysicalDevice(VkPhysicalDevice pDevice)
  {
    VkPhysicalDeviceProperties pdp = {};
    vkGetPhysicalDeviceProperties(pDevice, &pdp);

    bhLog_Message(LP_INFO,
      "Vulkan runtime [API] %d.%d.%d [Driver] %d.%d.%d [Device] %s (%s)",
      VK_VERSION_MAJOR(pdp.apiVersion), VK_VERSION_MINOR(pdp.apiVersion), VK_VERSION_PATCH(pdp.apiVersion),
      VK_VERSION_MAJOR(pdp.driverVersion), VK_VERSION_MINOR(pdp.driverVersion), VK_VERSION_PATCH(pdp.driverVersion),
      //pdp.vendorID,
      //pdp.deviceID,
      pdp.deviceName,
      GetPhysicalDeviceTypeString(pdp.deviceType)
      //pdp.pipelineCacheUUID,
      //pdp.limits,
      //pdp.sparseProperties
    );

    //VkPhysicalDeviceFeatures pdf = {};
    //vkGetPhysicalDeviceFeatures(pDevice, &pdf);

    //VkPhysicalDeviceMemoryProperties pdmp = {};
    //vkGetPhysicalDeviceMemoryProperties(pDevice, &pdmp);

    //uint32_t numQueueFamilies = 0;
    //vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &numQueueFamilies, nullptr);
    //std::vector<VkQueueFamilyProperties> queueFamilyProperties_v(numQueueFamilies);
    //vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &numQueueFamilies, queueFamilyProperties_v.data());
  }

  uint32_t GetDeviceTypeRank(VkPhysicalDeviceType type)
  {
    // See enum VkPhysicalDeviceType
    //VK_PHYSICAL_DEVICE_TYPE_OTHER = 0
    //VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 30
    //VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 40
    //VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 10
    //VK_PHYSICAL_DEVICE_TYPE_CPU = 20

    uint32_t deviceTypeRanks[] = { 0, 30, 40 ,10, 20 };
    return deviceTypeRanks[type];
  }

  VkImageType MapImageTypeToVK(int dim)
  {
    if ((0 < dim) && (dim < 4))
    {
      return (VkImageType)(dim - 1);
    }
    return VK_IMAGE_TYPE_MAX_ENUM; // Should be treated as invalid
  }

  ////////////////////////////////////////////////////////////////////////////////
  // If surface != VK_NULL_HANDLE, will try to get a queue with presentation support for this surface
  uint32_t GetQueueFamily(VkPhysicalDevice pDevice, const VkQueueFamilyProperties& requiredProperties, VkSurfaceKHR surface)
  {
    uint32_t numQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &numQueueFamilies, nullptr);
    if (numQueueFamilies > 0)
    {
      std::vector<VkQueueFamilyProperties> queueFamilyProperties_v(numQueueFamilies);
      vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &numQueueFamilies, queueFamilyProperties_v.data());

      for (uint32_t qfIdx = 0; qfIdx < numQueueFamilies; ++qfIdx)
      {
        VkBool32 result = VK_TRUE;
        result &= queueFamilyProperties_v[qfIdx].queueFlags && requiredProperties.queueFlags;
        result &= queueFamilyProperties_v[qfIdx].queueCount >= requiredProperties.queueCount;

        // TODO: So far we are looking for queues that support both rendering AND presentation
        // It could be that the device supports this functionality in separate queue families
        // so we are going to have to explicitly use outDevice->_renderQueueFamily, outDevice->_presentQueueFamily etc.

        if (surface != VK_NULL_HANDLE)
        {
          VkBool32 presentSupported = VK_FALSE;
          vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, qfIdx, surface, &presentSupported);
          result &= (presentSupported == VK_TRUE);
        }

        if (result)
        {
          return qfIdx;
        }
      }
    }
    return UINT32_MAX;
  }

  VkBool32 TestPhysicalDeviceExtensions(VkPhysicalDevice pDevice, const std::vector<const char*>& reqExtNames_v, const char* layerName)
  {
    uint32_t numAvailExtensions = 0, foundExtensions = 0;
    if (vkEnumerateDeviceExtensionProperties(pDevice, layerName, &numAvailExtensions, nullptr) != VK_SUCCESS)
    {
      return VK_FALSE;
    }
    std::vector<VkExtensionProperties> availExtensionProperties_v(numAvailExtensions);
    if (vkEnumerateDeviceExtensionProperties(pDevice, layerName, &numAvailExtensions, availExtensionProperties_v.data()) != VK_SUCCESS)
    {
      return VK_FALSE;
    }
    for (uint32_t re = 0; re < reqExtNames_v.size(); ++re)
    {
      for (uint32_t ae = 0; ae < numAvailExtensions; ++ae)
      {
        if (strcmp(reqExtNames_v[re], availExtensionProperties_v[ae].extensionName) == 0)
        {
          ++foundExtensions;
          break;
        }
      }
    }
    return (foundExtensions == reqExtNames_v.size()) ? VK_TRUE : VK_FALSE;
  }

  VkBool32 TestPhysicalDeviceSurfaceCaps(VkPhysicalDevice pDevice, VkSurfaceKHR surface, const VkSurfaceCapabilitiesKHR* reqCaps)
  {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &caps);
    VkBool32 result = VK_TRUE;

    result &= IsExtentValid2D(caps.currentExtent, reqCaps->currentExtent);
    result &= ((caps.currentTransform & reqCaps->currentTransform) == reqCaps->currentTransform);
    result &= (caps.maxImageArrayLayers >= reqCaps->maxImageArrayLayers);
    result &= (caps.maxImageCount >= reqCaps->maxImageCount);
    //result &= IsExtentValid2D(caps.maxImageExtent, reqCaps->maxImageExtent);
    result &= (caps.minImageCount <= reqCaps->minImageCount);
    //result &= IsExtentValid2D(reqCaps->minImageExtent, caps.minImageExtent);
    result &= ((caps.supportedCompositeAlpha & reqCaps->supportedCompositeAlpha) == reqCaps->supportedCompositeAlpha);
    result &= ((caps.supportedTransforms & reqCaps->supportedTransforms) == reqCaps->supportedTransforms);
    result &= ((caps.supportedUsageFlags & reqCaps->supportedUsageFlags) == reqCaps->supportedUsageFlags);

    return result;
  }

  uint32_t FindPhysicalDeviceMemoryTypeIndex(VkPhysicalDevice pDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags)
  {
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(pDevice, &physicalDeviceMemoryProperties);
    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
    {
      if ((typeFilter & (1 << i)) && ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & flags) == flags))
      {
        return i;
      }
    }
    return UINT32_MAX;
  }

  VkBool32 IsPushConstantRangeValid(VkPhysicalDevice pDevice, const VkPushConstantRange& range)
  {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(pDevice, &physicalDeviceProperties);

    VkBool32 result = VK_TRUE;

    result &= (range.offset <= physicalDeviceProperties.limits.maxPushConstantsSize);
    result &= (range.offset % 4) == 0;
    result &= (range.size > 0);
    result &= (range.size % 4) == 0;
    result &= (range.size <= (physicalDeviceProperties.limits.maxPushConstantsSize - range.offset));

    return result;
  }

  VkBool32 ArePushConstantRangesValid(VkPhysicalDevice pDevice, std::vector<VkPushConstantRange>& ranges_v)
  {
    for (VkPushConstantRange& r : ranges_v)
    {
      if (!IsPushConstantRangeValid(pDevice, r))
      {
        return VK_FALSE;
      }
    }
    return VK_TRUE;
  }

  VkBool32 TestImageFormat(VkPhysicalDevice pDevice, const VkImageCreateInfo& reqImageInfo)
  {
    VkBool32 result = VK_TRUE;
    VkImageFormatProperties ifp = {};
    vkGetPhysicalDeviceImageFormatProperties(pDevice, reqImageInfo.format, reqImageInfo.imageType, reqImageInfo.tiling, reqImageInfo.usage, reqImageInfo.flags, &ifp);

    result &= IsExtentValid3D(ifp.maxExtent, reqImageInfo.extent);
    result &= (ifp.maxMipLevels >= reqImageInfo.mipLevels);
    result &= (ifp.maxArrayLayers >= reqImageInfo.arrayLayers);
    //result &= (ifp.sampleCounts >= reqImageInfo.samples);
    //result &= (ifp.maxResourceSize >= reqImageInfo.);

    return result;
  }

  VkSurfaceFormatKHR DetermineSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
  {
    VkSurfaceFormatKHR sFormat = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
    uint32_t numFormats = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numFormats, nullptr) == VK_SUCCESS)
    {
      if (numFormats > 0)
      {
        std::vector<VkSurfaceFormatKHR> formats(numFormats);
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numFormats, formats.data()) == VK_SUCCESS)
        {
          sFormat = formats[0];
        }
      }
    }
    return sFormat;
  }

  VkBool32 IsPresentModeSupported(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode)
  {
    VkBool32 result = VK_FALSE;
    uint32_t numPresentModes = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numPresentModes, nullptr) == VK_SUCCESS)
    {
      if (numPresentModes > 0)
      {
        std::vector<VkPresentModeKHR> modes(numPresentModes);
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numPresentModes, modes.data()) == VK_SUCCESS)
        {
          for (uint32_t m = 0; (m < numPresentModes) && !result; ++m)
          {
            result = (modes[m] == presentMode);
          }
        }
      }
    }
    return result;
  }

  VkFormat PickImageFormat(const bhImage& img)
  {
    switch (img.numComponents)
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

  VkImageType MapImageTypeToVk(int dim)
  {
    if ((0 < dim) && (dim < 4))
    {
      return (VkImageType)(dim - 1);
    }
    return VK_IMAGE_TYPE_MAX_ENUM; // Should be treated as invalid
  }

  VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool)
  {
    VkCommandBufferAllocateInfo commandBufferAI =
    {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
      commandPool,
      VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      1
    };

    VkCommandBuffer outCommandBuffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &commandBufferAI, &outCommandBuffer) != VK_SUCCESS)
    {
      return VK_NULL_HANDLE;
    }
    VkCommandBufferBeginInfo commandBufferBI =
    {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    return (vkBeginCommandBuffer(outCommandBuffer, &commandBufferBI) == VK_SUCCESS) ? outCommandBuffer : VK_NULL_HANDLE;
  }

  void EndSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
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
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }

  bool IsTextureInputValid(const VkDescriptorImageInfo& dii)
  {
    return (dii.sampler != VK_NULL_HANDLE) && (dii.imageView != VK_NULL_HANDLE); /*&& (dii.imageLayout != VK_IMAGE_LAYOUT_UNDEFINED);*/
  }
}
