#include <string.h>
#include <assert.h>
#include "VK/bhVK.hpp"
#include "VK/bhInstanceVK.hpp"
#include "bhConfig.hpp"
#include "bhLog.hpp"
#include "bhEnv.hpp"

////////////////////////////////////////////////////////////////////////////////
VkBool32 bhInstanceVK::Init()
{
	if (volkInitialize() != VK_SUCCESS)
	{
		return VK_FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Init and test instance extensions
	std::vector<const char*> extensionNames_v;
	{
		// We always create graphics-enabled instance
		extensionNames_v.push_back("VK_KHR_surface");
	#if (_WIN32 || _WIN64)
		extensionNames_v.push_back("VK_KHR_win32_surface");
	//#elif
	//TODO:
	#endif

	#if BH_API_DEBUG
		extensionNames_v.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	#endif

		if (!bhVK::TestInstanceExtensions(extensionNames_v, nullptr))
		{
			return VK_FALSE;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// Init and test instance layers
	std::vector<const char*> layerNames_v;
	{
	#ifdef BH_API_DEBUG
		layerNames_v.push_back("VK_LAYER_KHRONOS_validation");
	#endif

		if (!bhVK::TestInstanceLayers(layerNames_v))
		{
			return VK_FALSE;
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
	instanceCI.enabledLayerCount = uint32_t(layerNames_v.size());
	instanceCI.ppEnabledLayerNames = layerNames_v.data();
	instanceCI.enabledExtensionCount = uint32_t(extensionNames_v.size());
	instanceCI.ppEnabledExtensionNames = extensionNames_v.data();

	if (vkCreateInstance(&instanceCI, allocator, &(instance)) == VK_SUCCESS)
	{
		volkLoadInstance(instance);
		return VK_TRUE;
	}
	return VK_FALSE;
}

void bhInstanceVK::Destroy()
{
	vkDestroyInstance(instance, allocator);
	instance = VK_NULL_HANDLE;
}

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

VkBool32 bhInstanceVK::InitDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCI = {};
	debugReportCallbackCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	//debugReportCallbackCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	debugReportCallbackCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugReportCallbackCI.pfnCallback = DebugCallbackFunc;

	return (vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCI, allocator, &debugReportCallback) == VK_SUCCESS) ? VK_TRUE : VK_FALSE;
}

void bhInstanceVK::DestroyDebugCallback()
{
	vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, allocator);
}

#endif //BH_API_DEBUG
