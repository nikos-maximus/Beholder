#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_assert.h>
#include <glm/mat4x4.hpp>
#include <stb_image.h>
#include "VK/bhVkRenderDevice.hpp"
#include "VK/bhVKFramebuffer.hpp"
#include "VK/bhVKTexture.hpp"
#include "bhConfig.hpp"
#include "bhSystem.hpp"
#include "bhUtil.hpp"
#include "bhLog.h"
#include "Platform/bhPlatform.hpp"
#include "Texture/bhImage.hpp"
#include "Texture/bhTextureCache.hpp"

#include "VK/bhVKWorldPipeline.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"

static bhTextureCache<bhVKTexture> s_textureCache;

bhVKRenderDevice::bhVKRenderDevice()
{
  //DEBUG
  clearValues_v[0].color.float32[0] = 1.0f;
  clearValues_v[0].color.float32[1] = 0.0f;
  clearValues_v[0].color.float32[2] = 1.0f;
  clearValues_v[0].color.float32[3] = 1.0f;

  clearValues_v[1].depthStencil.depth = 1.0f;
  clearValues_v[1].depthStencil.stencil = 0;
}

VkBool32 bhVKRenderDevice::Init(SDL_Window* wnd, const VkInstance inst, const VkAllocationCallbacks* instAlloc)
{
  SDL_assert(wnd && inst);

  SDL_Vulkan_CreateSurface(wnd, inst, &wndSurface);
  if (!CreateDevice(inst, instAlloc))
  {
    return VK_FALSE;
  }
  if (!CreatePresentRenderPass())
  {
    return VK_FALSE;
  }
  const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
  if (!CreateSwapchain(rs.num_swapchain_images))
  {
    return VK_FALSE;
  }

  SDL_assert(rs.num_swapchain_images == swapChain.frameBuffers_v.size());
  CreateCommandBuffers(rs.num_swapchain_images);
  vkGetDeviceQueue(device, gfxQueueFamilyIdx, 0, &renderQueue);

  // Create sync primitives
  VkFenceCreateInfo fenceCI = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  vkCreateFence(device, &fenceCI, allocCB, &drawFence); // TODO: Error check

  VkSemaphoreCreateInfo semaphoreCI = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  vkCreateSemaphore(device, &semaphoreCI, allocCB, &sph_ImageAvailable); // TODO: Error check
  vkCreateSemaphore(device, &semaphoreCI, allocCB, &sph_RenderFinished); // TODO: Error check

  //g_clearRects_v[0] = {};
  //g_clearRects_v[0].rect.extent.width = bhConfig::GetWidth();
  //g_clearRects_v[0].rect.extent.height = bhConfig::GetHeight();

  s_textureCache.Init();

  return VK_TRUE;
}

void bhVKRenderDevice::Destroy(const VkInstance inst, const VkAllocationCallbacks* instAllocator)
{
  for (auto& tx : s_textureCache.GetTextures())
  {
    DestroyTexture(tx.second);
  }

  //textureManager.DestroyAll([this](bhTextureVK* t)
  //	{
  //		this->DestroyTexture(t);
  //	});

  // Destroy sync primitives
  vkDestroySemaphore(device, sph_RenderFinished, allocCB);
  vkDestroySemaphore(device, sph_ImageAvailable, allocCB);

  vkDestroyFence(device, drawFence, allocCB);

  vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers_v.size()), commandBuffers_v.data());
  commandBuffers_v.clear();

  DestroySwapchain();
  vkDestroyRenderPass(device, presentRenderPass, allocCB);

  vkDeviceWaitIdle(device);

  vkDestroyDescriptorPool(device, descriptorPool, allocCB);
  vkDestroyCommandPool(device, commandPool, allocCB);
  vkDestroyDevice(device, allocCB);

  vkDestroySurfaceKHR(inst, wndSurface, instAllocator);
  wndSurface = VK_NULL_HANDLE;
}

VkBool32 bhVKRenderDevice::CreateDevice(const VkInstance inst, const VkAllocationCallbacks* instAllocator)
{
  VkQueueFamilyProperties reqQFP_Graphics = {};
  reqQFP_Graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT;
  reqQFP_Graphics.queueCount = 1;

  std::vector<const char*> reqExtNames_v = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
  if (!ChoosePhysicalRenderDevice(inst, reqQFP_Graphics, reqExtNames_v))
  {
    return VK_FALSE;
  }

  VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
  vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
  //TODO - we should provide only the required features

  const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
  physicalDeviceFeatures.samplerAnisotropy = (rs.anisotropy_level > 0);

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

  if (vkCreateDevice(physicalDevice, &deviceCI, instAllocator, &device) == VK_SUCCESS)
  {
    VkCommandPoolCreateInfo commandPoolCreateInfo =
    {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      gfxQueueFamilyIdx
    };

    if (vkCreateCommandPool(device, &commandPoolCreateInfo, allocCB, &commandPool) == VK_SUCCESS)
    {
      return CreateDescriptorPool();
      //return true;
    }
  }
  return VK_FALSE;
}

VkBool32 bhVKRenderDevice::ChoosePhysicalRenderDevice(const VkInstance inst, const VkQueueFamilyProperties& reqQFP_Graphics, const std::vector<const char*>& reqExtNames_v)
{
  uint32_t numPhysicalDevices = 0;
  if (vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, nullptr) != VK_SUCCESS)
  {
    // TODO: Log Error?
    return VK_FALSE;
  }
  std::vector<VkPhysicalDevice> physicalDevices_v(numPhysicalDevices);
  if (vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, physicalDevices_v.data()) != VK_SUCCESS)
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

    queueFamilyInds_v[i] = bhVK::GetQueueFamily(physicalDevices_v[i], reqQFP_Graphics, wndSurface);
    if (queueFamilyInds_v[i] == UINT32_MAX)
    {
      continue;
    }
    if (bhVK::TestPhysicalDeviceExtensions(physicalDevices_v[i], reqExtNames_v, nullptr) == false)
    {
      continue;
    }

    ranks_v[i] += RankPhysicalDeviceGraphicsFeatures(physicalDevices_v[i]);
    ranks_v[i] += bhVK::GetDeviceTypeRank(pdp.deviceType);
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
    bhVK::LogPhysicalDevice(physicalDevice);
    return VK_TRUE;
  }
  return VK_FALSE;
}

uint32_t bhVKRenderDevice::RankPhysicalDeviceGraphicsFeatures(const VkPhysicalDevice physicalDevice)
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

VkBool32 bhVKRenderDevice::CreateDescriptorPool()
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

  VkResult result = vkCreateDescriptorPool(device, &descriptorPoolCI, allocCB, &descriptorPool);
  return (result == VK_SUCCESS) ? VK_TRUE : VK_FALSE;
}

VkBool32 bhVKRenderDevice::CreatePresentRenderPass()
{
  std::vector<VkAttachmentDescription> attachmentDescriptions_v(2);
  {
    attachmentDescriptions_v[0].flags = 0;
    attachmentDescriptions_v[0].format = bhVK::DetermineSurfaceFormat(physicalDevice, wndSurface).format;
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

  if (vkCreateRenderPass(device, &renderPassCI, allocCB, &presentRenderPass) == VK_SUCCESS)
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


//bhVKFramebuffer bhVKRenderDevice::CreateFramebuffer(VkExtent2D size, uint32_t numColorAttachments, bool createDepthStencil)
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

VkBool32 bhVKRenderDevice::CreateSwapchain(uint32_t numImages)
{
  // Returns the number of swap chain images, so 0 meeans failure
  VkSurfaceFormatKHR surfaceFormat = bhVK::DetermineSurfaceFormat(physicalDevice, wndSurface);
  if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
  {
    return VK_FALSE;
  }

  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  if (!bhVK::IsPresentModeSupported(physicalDevice, wndSurface, presentMode))
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

  if (!bhVK::TestPhysicalDeviceSurfaceCaps(physicalDevice, wndSurface, &surfaceCapabilities))
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
  if (vkCreateSwapchainKHR(device, &swapChainCI, allocCB, &(swapChain.swapChain)) == VK_SUCCESS)
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
          swapChain.frameBuffers_v[fbIdx].Create(this, wndExtent, swapChainColorBuffers_v.data()[fbIdx], surfaceFormat, true);
        }
      }
    }
  }
  return (numSwapChainImages > 0) ? VK_TRUE : VK_FALSE;
}

void bhVKRenderDevice::DestroySwapchain()
{
  for (size_t fb = 0; fb < swapChain.frameBuffers_v.size(); ++fb)
  {
    swapChain.frameBuffers_v[fb].Destroy(this);
  }
  //g_swapChainImages_v are handled by the swapChain, and destroyed along with it
  vkDestroySwapchainKHR(device, swapChain.swapChain, allocCB);
  swapChain.Clear();
}

VkBool32 bhVKRenderDevice::CreateCommandBuffers(uint32_t numBuffers)
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

const bhVKTexture* bhVKRenderDevice::GetTexture(bhHash_t hash) const
{
  const bhVKTexture* tex = s_textureCache.GetTexture(hash);
  if (tex)
  {
    return tex;
  }

  const std::string& path = s_textureCache.GetPath(hash);
  bhImage* newImg = bhImage::CreateFromFile(path.c_str(), STBI_rgb_alpha);
  if (newImg)
  {
    bhVKTexture newTex = CreateTextureFromImage(*newImg, 0, 0);
    s_textureCache.AddTexture(hash, newTex);
    return s_textureCache.GetTexture(hash);
  }
  return nullptr;
}

const bhVKTexture* bhVKRenderDevice::GetTexture(const char* name) const
{
  return GetTexture(bhHash(name));
}

bhVKTexture bhVKRenderDevice::CreateTexture(const VkImageCreateInfo& imageCI) const
{
  VkImage txImage = VK_NULL_HANDLE;
  if (vkCreateImage(device, &imageCI, allocCB, &txImage) == VK_SUCCESS)
  {
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, txImage, &memoryRequirements);

    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t memTypeIdx = bhVK::FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
    assert(memTypeIdx < UINT32_MAX);

    VkMemoryAllocateInfo memoryAllocateInfo =
    {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
      memoryRequirements.size, //allocationSize
      memTypeIdx //memoryTypeIndex
    };

    VkDeviceMemory txMemory = VK_NULL_HANDLE;
    if (vkAllocateMemory(device, &memoryAllocateInfo, allocCB, &txMemory) == VK_SUCCESS)
    {
      vkBindImageMemory(device, txImage, txMemory, 0);
      bhVKTexture::Properties props = { txImage, txMemory, imageCI.format, imageCI.extent, 1 };
      return bhVKTexture(props);
    }
    vkDestroyImage(device, txImage, allocCB);
  }
  return {};
}

VkShaderModule bhVKRenderDevice::CreateShaderFromFile(const char* fileName) const
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

    VkResult result = vkCreateShaderModule(device, &shaderModuleCI, allocCB, &newShaderMd);
    bhUtil::FreeFileData(fileData);
  }
  bhPlatform::FreePath(path);
  assert(newShaderMd != VK_NULL_HANDLE);
  return newShaderMd;
}

void bhVKRenderDevice::BeginFrame()
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
    swapChain.frameBuffers_v[swapChain.swapChainIdx].Size()
  };

  VkRenderPassBeginInfo renderPassBI =
  {
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr,
    presentRenderPass,
    swapChain.frameBuffers_v[swapChain.swapChainIdx].Framebuffer(),
    renderArea,
    2, clearValues_v
  };

  vkCmdBeginRenderPass(commandBuffers_v[swapChain.swapChainIdx], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
}

void bhVKRenderDevice::EndFrame()
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

void bhVKRenderDevice::DestroyPipeline(VkPipeline pipeline) const
{
  vkDestroyPipeline(device, pipeline, allocCB);
}

bhVKTexture bhVKRenderDevice::CreateMemoryBackedImage(const VkImageCreateInfo& imageCI) const
{
  VkImage txImage = VK_NULL_HANDLE;
  if (vkCreateImage(device, &imageCI, allocCB, &txImage) != VK_SUCCESS)
  {
    return {};
  }

  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(device, txImage, &memoryRequirements);

  VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  uint32_t memTypeIdx = bhVK::FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
  assert(memTypeIdx < UINT32_MAX);

  VkMemoryAllocateInfo memoryAllocateInfo =
  {
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
    memoryRequirements.size, //allocationSize
    memTypeIdx //memoryTypeIndex
  };

  VkDeviceMemory txMemory = VK_NULL_HANDLE;
  if (vkAllocateMemory(device, &memoryAllocateInfo, allocCB, &txMemory) == VK_SUCCESS)
  {
    vkBindImageMemory(device, txImage, txMemory, 0);
    bhVKTexture::Properties props = { txImage, txMemory, imageCI.format, imageCI.extent, 1 };
    return bhVKTexture(props);
  }
  vkDestroyImage(device, txImage, allocCB);

  return {};
}

bhVKTexture bhVKRenderDevice::CreateTexture(VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels) const
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

  return CreateMemoryBackedImage(imageCI);
}

void bhVKRenderDevice::DestroyTexture(bhVKTexture& texture) const
{
  vkFreeMemory(device, texture.Memory(), allocCB);
  vkDestroyImage(device, texture.Image(), allocCB);
  texture = {};
}

bhVK::Buffer bhVKRenderDevice::CreateBufferFromImage(const bhImage& img) const
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

  bhVK::Buffer newBuffer;
  if (vkCreateBuffer(device, &bufferCreateInfo, allocCB, &(newBuffer.buffer)) != VK_SUCCESS)
  {
    return {};
  }

  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device, newBuffer.buffer, &memoryRequirements);

  VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  uint32_t memTypeIdx = bhVK::FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, flags);
  assert(memTypeIdx < UINT32_MAX);

  VkMemoryAllocateInfo memoryAllocateInfo =
  {
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
      memoryRequirements.size,
      memTypeIdx
  };

  if (vkAllocateMemory(device, &memoryAllocateInfo, allocCB, &(newBuffer.memory)) != VK_SUCCESS)
  {
    vkDestroyBuffer(device, newBuffer.buffer, allocCB);
    return {};
  }

  newBuffer.size = memoryRequirements.size;
  vkBindBufferMemory(device, newBuffer.buffer, newBuffer.memory, 0);
  void* data;
  vkMapMemory(device, newBuffer.memory, 0, imageSize, 0, &data);
  memcpy(data, img.pixels, imageSize);
  vkUnmapMemory(device, newBuffer.memory);

  return newBuffer;
}

bhVKTexture bhVKRenderDevice::CreateTextureFromImage(const bhImage& img, int32_t offsetx, int32_t offsety) const
{
  // Try to make sense out of img components
  VkFormat fmt = bhVK::PickImageFormat(img);
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
  bhVKTexture newTex = CreateTexture(bhVK::MapImageTypeToVk(bhImage::DetermineDimensions(img)), fmt, extent, requiredMipLevels);
  if (!newTex.IsValid())
  {
    return {};
  }

  // Create the buffer from pixel data to copy the texture img from
  bhVK::Buffer imageBuffer = CreateBufferFromImage(img);
  if (!imageBuffer.IsValid())
  {
    return {};
  }

  VkCommandBuffer commandBuffer = bhVK::BeginSingleTimeCommandBuffer(device, commandPool);
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
      newTex.Image(), //image
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
    vkCmdCopyBufferToImage(commandBuffer, imageBuffer.buffer, newTex.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

    const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
    if (rs.GenMipmaps())
    {
      newTex.GenerateMipmaps(commandBuffer, requiredMipLevels);
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

    bhVK::EndSingleTimeCommandBuffer(device, commandPool, commandBuffer, renderQueue);
  }
  // Clear intermediate img source buffer
  DestroyBuffer(imageBuffer);
  return newTex;
}

VkDescriptorImageInfo bhVKRenderDevice::SetupTextureDescriptor(const bhVKTexture& tx) const
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

  VkResult res = vkCreateSampler(device, &samplerCI, allocCB, &descriptorImgInfo.sampler);
  if (res != VK_SUCCESS)
  {
    // Report VK_ERROR_OUT_OF_HOST_MEMORY / VK_ERROR_OUT_OF_DEVICE_MEMORY
  }
  return descriptorImgInfo;
}

VkImageView bhVKRenderDevice::CreateTextureImageView(const bhVKTexture& tx) const
{
  if (tx.Format() == VK_FORMAT_UNDEFINED)
  {
    return VK_NULL_HANDLE;
  }

  VkComponentMapping componentMapping = {};

  VkImageSubresourceRange imageSubresourceRange =
  {
    VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
    0, //baseMipLevel
    tx.MipLevels(), //levelCount
    0, //baseArrayLayer
    1 //layerCount
  };

  VkImageViewCreateInfo imageViewCI =
  {
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
    tx.Image(),
    VK_IMAGE_VIEW_TYPE_2D,
    tx.Format(),
    componentMapping,
    imageSubresourceRange
  };

  VkImageView imageView = VK_NULL_HANDLE;
  vkCreateImageView(device, &imageViewCI, allocCB, &imageView);
  return imageView;
}

float bhVKRenderDevice::GetAnisotropy() const
{
  VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
  vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
  if (physicalDeviceFeatures.samplerAnisotropy)
  {
    const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
    return (rs.anisotropy_level == 0) ? 0.0f : float(1 << rs.anisotropy_level);
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

bool bhVKRenderDevice::InitImGui(SDL_Window* wnd, VkInstance* instance) const
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
    initInfo.Instance = *instance;
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
    initInfo.Allocator = allocCB;
    initInfo.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
      return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
      }, instance);
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
    return true;
  }
  return false;
}

void bhVKRenderDevice::DestroyImGui() const
{
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

void bhVKRenderDevice::BeginImGuiFrame() const
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void bhVKRenderDevice::EndImGuiFrame() const
{
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers_v[swapChain.swapChainIdx]);
}
