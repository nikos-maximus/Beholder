#define VOLK_IMPLEMENTATION

#include "VK/bhStructsVK.hpp"
#include "bhLog.h"
#include "bhConfig.h"
#include "bhEnv.h"
#include "bhMesh.hpp"

////////////////////////////////////////////////////////////////////////////////
#ifdef BH_API_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallbackFunc(
	VkDebugReportFlagsEXT messageSeverity,
	VkDebugReportObjectTypeEXT /*messageType*/,
	uint64_t /*obj*/,
	size_t /*location*/,
	int32_t /*code*/,
	const char* /*layerPrefix*/,
	const char* msg,
	void* /*userData*/)
{
	//TODO check flags and differentiate for info/warning/error etc.
	switch (messageSeverity)
	{
		case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		{
			bhLog_Message(LOG_TYPE_DEBUG, "%s", msg);
			break;
		}
		case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		{
			bhLog_Message(LOG_TYPE_ERROR, "%s", msg);
			break;
		}
		case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		{
			bhLog_Message(LOG_TYPE_PERFORMANCE, "[PERFORMANCE] %s", msg);
			break;
		}
		case VK_DEBUG_REPORT_WARNING_BIT_EXT:
		{
			bhLog_Message(LOG_TYPE_WARNING, "%s", msg);
			break;
		}
		case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		default:
		{
			bhLog_Message(LOG_TYPE_INFO, "%s", msg);
			break;
		}
	}
	return VK_FALSE;
}

static VkDebugReportCallbackEXT debugReportCallback;

VkBool32 bhVK_InitDebugCallback(const bhInstanceVK* pInstance)
{
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCI = {};
	debugReportCallbackCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	//debugReportCallbackCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	debugReportCallbackCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugReportCallbackCI.pfnCallback = DebugCallbackFunc;

	return (vkCreateDebugReportCallbackEXT(pInstance->instance, &debugReportCallbackCI, pInstance->allocator, &debugReportCallback) == VK_SUCCESS) ? VK_TRUE : VK_FALSE;
}

void bhVK_DestroyDebugCallback(const bhInstanceVK* pInstance)
{
	vkDestroyDebugReportCallbackEXT(pInstance->instance, debugReportCallback, pInstance->allocator);
}

#endif //BH_API_DEBUG

////////////////////////////////////////////////////////////////////////////////
VkBool32 TestInstanceExtensions(uint32_t numRreqExtensionNames, const char** const reqExtensionNames_v, const char* layerName);
VkBool32 TestInstanceLayers(uint32_t numReqLayerNames, const char** const reqLayerNames_v);

bhInstanceVK bhVK_InitInstance()
{
	if (volkInitialize() != VK_SUCCESS)
	{
		return {};
	}

	////////////////////////////////////////////////////////////////////////////////
	// Init and test instance extensions
	constexpr uint32_t MAX_EXTENSIONS = 3;
	uint32_t numExtensions = 0;
	const char* extensionNames_v[MAX_EXTENSIONS];
	{
		int32_t extIdx = -1;
		// We always create graphics-enabled instance
		extensionNames_v[++extIdx] = "VK_KHR_surface";
	#if (_WIN32 || _WIN64)
		extensionNames_v[++extIdx] = "VK_KHR_win32_surface";
		//#elif
			// TODO:
	#endif

	#if BH_API_DEBUG
		extensionNames_v[++extIdx] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
	#endif
		assert(extIdx < MAX_EXTENSIONS);

		numExtensions = extIdx + 1;
		if (!TestInstanceExtensions(numExtensions, extensionNames_v, nullptr))
		{
			return {};
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// Init and test instance layers
	constexpr uint32_t MAX_LAYERS = 1;
	uint32_t numLayers = 0;
	const char* layerNames_v[MAX_LAYERS];
	{
		int32_t layerIdx = -1;
	#ifdef BH_API_DEBUG
		layerNames_v[++layerIdx] = "VK_LAYER_KHRONOS_validation";
	#endif

		numLayers = layerIdx + 1;
		if (!TestInstanceLayers(numLayers, layerNames_v))
		{
			return {};
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// Create instance
	const bhRenderSettings* rs = bhConfig_GetRenderSettings();

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	//applicationInfo.apiVersion = ;
	applicationInfo.applicationVersion = bhEnv_GetApplicationVersion();
	applicationInfo.engineVersion = bhEnv_GetEngineVersion();
	applicationInfo.pApplicationName = bhEnv_GetApplicationName();
	applicationInfo.pEngineName = bhEnv_GetEngineName();
	applicationInfo.apiVersion = VK_MAKE_VERSION(rs->vk.versionMajor, rs->vk.versionMinor, rs->vk.versionPatch);

	bhLog_Message(LOG_TYPE_INFO, "Vulkan version requested: %d.%d.%d", rs->vk.versionMajor, rs->vk.versionMinor, rs->vk.versionPatch);

	VkInstanceCreateInfo instanceCI = {};
	instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCI.pApplicationInfo = &applicationInfo;
	instanceCI.enabledLayerCount = numLayers;
	instanceCI.ppEnabledLayerNames = layerNames_v;
	instanceCI.enabledExtensionCount = numExtensions;
	instanceCI.ppEnabledExtensionNames = extensionNames_v;

	bhInstanceVK newInstance = {};
	if (vkCreateInstance(&instanceCI, newInstance.allocator, &(newInstance.instance)) == VK_SUCCESS)
	{
		volkLoadInstance(newInstance.instance);
		return newInstance;
	}
	return {};
}

void bhVK_DestroyInstance(bhInstanceVK* pInstance)
{
	vkDestroyInstance(pInstance->instance, pInstance->allocator);
	pInstance->instance = VK_NULL_HANDLE;
}

VkBool32 TestInstanceLayers(uint32_t numReqLayerNames, const char** const reqLayerNames_v)
{
	uint32_t numLayerProperties = 0, numFoundLayers = 0;
	VkResult result = vkEnumerateInstanceLayerProperties(&numLayerProperties, nullptr);
	if ((result == VK_SUCCESS) && (numLayerProperties > 0))
	{
		VkLayerProperties* layerProperties_v = (VkLayerProperties*)calloc(numLayerProperties, sizeof(VkLayerProperties));
		result = vkEnumerateInstanceLayerProperties(&numLayerProperties, layerProperties_v);
		if (result == VK_SUCCESS)
		{
			// bhLog_Message(LT_INFO, "---------- Vulkan Instance Layers ----------");
			for (uint32_t lp = 0; lp < numLayerProperties; ++lp)
			{
				// bhLog_Message(LT_INFO, "%s (%s) - Spec. %d", layerProperties_v[lp].layerName, layerProperties_v[lp].description, layerProperties_v[lp].specVersion);
				for (size_t rl = 0; rl < numReqLayerNames; ++rl)
				{
					if (strcmp(reqLayerNames_v[rl], layerProperties_v[lp].layerName) == 0)
					{
						++numFoundLayers;
						break;
					}
				}
			}
			// bhLog_Message(LT_INFO, "--------------------------------------------");
		}
		free(layerProperties_v);
	}
	return (numReqLayerNames == numFoundLayers) ? VK_TRUE : VK_FALSE;
}

VkBool32 TestInstanceExtensions(uint32_t numRreqExtensionNames, const char** const reqExtensionNames_v, const char* layerName)
{
	uint32_t numExtensionProperties = 0, numFoundExtensions = 0;

	VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &numExtensionProperties, nullptr);
	if ((result == VK_SUCCESS) && (numExtensionProperties > 0))
	{
		VkExtensionProperties* extensionProperties_v = (VkExtensionProperties*)calloc(numExtensionProperties, sizeof(VkExtensionProperties));
		result = vkEnumerateInstanceExtensionProperties(layerName, &numExtensionProperties, extensionProperties_v);
		if (result == VK_SUCCESS)
		{
			// bhLog_Message(LT_INFO, "---------- Vulkan Instance Extensions ----------");
			for (uint32_t ep = 0; ep < numExtensionProperties; ++ep)
			{
				// bhLog_Message(LT_INFO, "%s - Spec. %d", extensionProperties_v[ep].extensionName, extensionProperties_v[ep].specVersion);
				for (size_t re = 0; re < numRreqExtensionNames; ++re)
				{
					if (strcmp(reqExtensionNames_v[re], extensionProperties_v[ep].extensionName) == 0)
					{
						++numFoundExtensions;
						break;
					}
				}
			}
			// bhLog_Message(LT_INFO, "------------------------------------------------");
		}
		free(extensionProperties_v);
	}
	return (numRreqExtensionNames == numFoundExtensions) ? VK_TRUE : VK_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
int bhVK_GetDeviceTypeRank(VkPhysicalDeviceType type)
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

// If surface != VK_NULL_HANDLE, will try to get a queue with presentation support for this surface
uint32_t bhVK_GetQueueFamily(VkPhysicalDevice physicalDevice, const VkQueueFamilyProperties* requiredProperties, VkSurfaceKHR surface)
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, nullptr);
	if (numQueueFamilies > 0)
	{
		std::vector<VkQueueFamilyProperties> queueFamilyProperties_v(numQueueFamilies);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, queueFamilyProperties_v.data());

		for (uint32_t qfIdx = 0; qfIdx < numQueueFamilies; ++qfIdx)
		{
			VkBool32 result = VK_TRUE;
			result &= queueFamilyProperties_v[qfIdx].queueFlags && requiredProperties->queueFlags;
			result &= queueFamilyProperties_v[qfIdx].queueCount >= requiredProperties->queueCount;

			// TODO: So far we are looking for queues that support both rendering AND presentation
			// It could be that the device supports this functionality in separate queue families
			// so we are going to have to explicitly use outDevice->_renderQueueFamily, outDevice->_presentQueueFamily etc.

			if (surface != VK_NULL_HANDLE)
			{
				VkBool32 presentSupported = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, qfIdx, surface, &presentSupported);
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

VkBool32 bhVK_TestPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice, const std::vector<const char*>* reqExtNames_v, const char* layerName)
{
	uint32_t numAvailExtensions = 0, foundExtensions = 0;
	if (vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &numAvailExtensions, nullptr) != VK_SUCCESS)
	{
		return VK_FALSE;
	}
	std::vector<VkExtensionProperties> availExtensionProperties_v(numAvailExtensions);
	if (vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &numAvailExtensions, availExtensionProperties_v.data()) != VK_SUCCESS)
	{
		return VK_FALSE;
	}
	for (uint32_t re = 0; re < reqExtNames_v->size(); ++re)
	{
		for (uint32_t ae = 0; ae < numAvailExtensions; ++ae)
		{
			if (strcmp((*reqExtNames_v)[re], availExtensionProperties_v[ae].extensionName) == 0)
			{
				++foundExtensions;
				break;
			}
		}
	}
	return (foundExtensions == reqExtNames_v->size()) ? VK_TRUE : VK_FALSE;
}

const char* bhVK_GetPhysicalDeviceTypeString(VkPhysicalDeviceType physicalDeviceType)
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

void bhVK_LogPhysicalDevice(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties pdp = {};
	vkGetPhysicalDeviceProperties(physicalDevice, &pdp);

	bhLog_Message(LOG_TYPE_INFO,
		"Vulkan runtime [API] %d.%d.%d [Driver] %d.%d.%d [Device] %s (%s)",
		VK_VERSION_MAJOR(pdp.apiVersion), VK_VERSION_MINOR(pdp.apiVersion), VK_VERSION_PATCH(pdp.apiVersion),
		VK_VERSION_MAJOR(pdp.driverVersion), VK_VERSION_MINOR(pdp.driverVersion), VK_VERSION_PATCH(pdp.driverVersion),
		//pdp.vendorID,
		//pdp.deviceID,
		pdp.deviceName,
		bhVK_GetPhysicalDeviceTypeString(pdp.deviceType)
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

VkSurfaceFormatKHR bhVK_DetermineSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceFormatKHR sFormat = {};
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

VkBool32 bhVK_IsPresentModeSupported(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode)
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

inline VkBool32 bhVK_IsExtentValid2D(const VkExtent2D& physicalDeviceExtent, const VkExtent2D& reqExtent)
{
	return (physicalDeviceExtent.width >= reqExtent.width) && (physicalDeviceExtent.height >= reqExtent.height) ? VK_TRUE : VK_FALSE;
}

inline VkBool32 bhVK_IsExtentValid3D(const VkExtent3D& physicalDeviceExtent, const VkExtent3D& reqExtent)
{
	return (physicalDeviceExtent.width >= reqExtent.width) && (physicalDeviceExtent.height >= reqExtent.height) && (physicalDeviceExtent.depth >= reqExtent.depth) ? VK_TRUE : VK_FALSE;
}

VkBool32 bhVK_TestPhysicalDeviceSurfaceCaps(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const VkSurfaceCapabilitiesKHR* reqCaps)
{
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
	VkBool32 result = VK_TRUE;
	
	result &= bhVK_IsExtentValid2D(caps.currentExtent, reqCaps->currentExtent);
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

std::vector<VkCommandBuffer> bhVK_CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t numBuffers)
{
	VkCommandBufferAllocateInfo commandBufferAI =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		numBuffers
	};
	
	std::vector<VkCommandBuffer> commandBuffers_v(numBuffers);
	VkResult res = vkAllocateCommandBuffers(device, &commandBufferAI, commandBuffers_v.data());
	assert(res == VK_SUCCESS);
	return std::move(commandBuffers_v);
}

void bhVK_DestroyCommandBuffers(std::vector<VkCommandBuffer>* commandBuffers_v, VkDevice device, VkCommandPool commandPool)
{
	vkFreeCommandBuffers(device, commandPool, (uint32_t)commandBuffers_v->size(), commandBuffers_v->data());
	commandBuffers_v->clear();
}

VkFence bhVK_CreateFence(VkDevice device,const VkAllocationCallbacks* allocator)
{
	VkFenceCreateInfo fenceCI =
	{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0
	};

	VkFence newFence = VK_NULL_HANDLE;
	VkResult ret = vkCreateFence(device, &fenceCI, allocator, &newFence);
	if (ret != VK_SUCCESS)
	{
		// Report VK_ERROR_OUT_OF_HOST_MEMORY / VK_ERROR_OUT_OF_DEVICE_MEMORY
	}
	return newFence;
}

void bhVK_DestroyFence(VkFence fence, VkDevice device, const VkAllocationCallbacks* allocator)
{
	vkDestroyFence(device, fence, allocator);
}

////////////////////////////////////////////////////////////////////////////////
uint32_t bhVK_FindPhysicalDeviceMemoryTypeIndex(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physDevice, &physicalDeviceMemoryProperties);
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & flags) == flags))
		{
			return i;
		}
	}
	return UINT32_MAX;
}

////////////////////////////////////////////////////////////////////////////////
VkBool32 bhVK_IsPushConstantRangeValid(VkPhysicalDevice physDevice, const VkPushConstantRange* range)
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physDevice, &physicalDeviceProperties);
	
	VkBool32 result = VK_TRUE;

	result &= (range->offset <= physicalDeviceProperties.limits.maxPushConstantsSize);
	result &= (range->offset % 4) == 0;
	result &= (range->size > 0);
	result &= (range->size % 4) == 0;
	result &= (range->size <= (physicalDeviceProperties.limits.maxPushConstantsSize - range->offset));
	
	return result;
}

VkBool32 bhVK_ArePushConstantRangesValid(VkPhysicalDevice physDevice, std::vector<VkPushConstantRange>* ranges_v)
{
	if(!ranges_v)
	{
		return VK_FALSE;
	}
	for (VkPushConstantRange& r : *ranges_v)
	{
		if (!bhVK_IsPushConstantRangeValid(physDevice, &r))
		{
			return VK_FALSE;
		}
	}
	return VK_TRUE;
}

////////////////////////////////////////////////////////////////////////////////
VkCommandBuffer bhVK_BeginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool)
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

void bhVK_EndSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
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

void bhVK_CreateUniformBufferBinding(VkDevice device, VkDescriptorSet descSet, VkBuffer buffer, uint32_t binding)
{
	assert(descSet != VK_NULL_HANDLE);

	VkDescriptorBufferInfo bufferInfo = { buffer, 0, VK_WHOLE_SIZE };
	VkWriteDescriptorSet writeDescSet =
	{
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
		descSet,
		binding,
		0,
		1,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		nullptr,
		&bufferInfo,
		nullptr
	};
	vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);
}

VkBool32 bhVK_TestImageFormat(VkPhysicalDevice physicalDevice, const VkImageCreateInfo& reqImageInfo)
{
	VkBool32 result = VK_TRUE;
	VkImageFormatProperties ifp = {};
	vkGetPhysicalDeviceImageFormatProperties(physicalDevice, reqImageInfo.format, reqImageInfo.imageType, reqImageInfo.tiling, reqImageInfo.usage, reqImageInfo.flags, &ifp);

	result &= bhVK_IsExtentValid3D(ifp.maxExtent, reqImageInfo.extent);
	result &= (ifp.maxMipLevels >= reqImageInfo.mipLevels);
	result &= (ifp.maxArrayLayers >= reqImageInfo.arrayLayers);
	//result &= (ifp.sampleCounts >= reqImageInfo.samples);
	//result &= (ifp.maxResourceSize >= reqImageInfo.);

	return result;
}

VkShaderStageFlagBits bhVK_GetShaderType(bhShaderType shaderType)
{
	VkShaderStageFlagBits types[NUM_SHADER_TYPES] =
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
