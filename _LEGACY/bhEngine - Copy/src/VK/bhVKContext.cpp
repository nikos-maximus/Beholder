#include <SDL3/SDL_video.h>
#include <vector>
#include "VK/bhVKContext.hpp"
#include "VK/bhVK.hpp"
#include "VK/bhVKWorldPipeline.hpp"
#include "bhSystem.hpp"
#include "bhConfig.hpp"
#include "bhLog.h"
#include "Mesh/bhMesh.hpp"

//DEBUG
#include <assert.h>
//DEBUG

VkBool32 TestInstanceExtensions(const std::vector<const char*>& extensionNames_v, const char* layerName)
{
  uint32_t numExtensionProperties = 0;
  size_t numFoundExtensions = 0;

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
        for (size_t re = 0; re < extensionNames_v.size(); ++re)
        {
          if (strcmp(extensionNames_v[re], extensionProperties_v[ep].extensionName) == 0)
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
  return (extensionNames_v.size() == numFoundExtensions) ? VK_TRUE : VK_FALSE;
}

VkBool32 TestInstanceLayers(const std::vector<const char*>& layerNames_v)
{
  uint32_t numLayerProperties = 0;
  size_t numFoundLayers = 0;

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
        for (size_t rl = 0; rl < layerNames_v.size(); ++rl)
        {
          if (strcmp(layerNames_v[rl], layerProperties_v[lp].layerName) == 0)
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
  return (layerNames_v.size() == numFoundLayers) ? VK_TRUE : VK_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
Uint32 bhVKContext::GetWindowFlag()
{
  return SDL_WINDOW_VULKAN;
}

VkBool32 bhVKContext::InitExtensionFunctions()
{
  VkBool32 result = VK_TRUE;

  ctxInst.CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(ctxInst.instance, "vkCreateDebugReportCallbackEXT");
  result &= ctxInst.CreateDebugReportCallbackEXT != nullptr;

  ctxInst.DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(ctxInst.instance, "vkDestroyDebugReportCallbackEXT");
  result &= ctxInst.DestroyDebugReportCallbackEXT != nullptr;

  return result;
}

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
      bhLog_Message(LP_DEBUG, "%s", msg);
      break;
    }
    case VK_DEBUG_REPORT_ERROR_BIT_EXT:
    {
      bhLog_Message(LP_ERROR, "%s", msg);
      break;
    }
    case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
    case VK_DEBUG_REPORT_WARNING_BIT_EXT:
    {
      bhLog_Message(LP_WARN, "%s", msg);
      break;
    }
    case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
    default:
    {
      bhLog_Message(LP_INFO, "%s", msg);
      break;
    }
  }
  return VK_FALSE;
}

VkBool32 bhVKContext::InitDebugCallback()
{
  VkDebugReportCallbackCreateInfoEXT debugReportCallbackCI = {};
  debugReportCallbackCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  //debugReportCallbackCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
  debugReportCallbackCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  debugReportCallbackCI.pfnCallback = DebugCallbackFunc;

  return (ctxInst.CreateDebugReportCallbackEXT(ctxInst.instance, &debugReportCallbackCI, ctxInst.allocator, &(ctxInst.debugReportCallback)) == VK_SUCCESS) ? VK_TRUE : VK_FALSE;
}

VkBool32 bhVKContext::InitInstance()
{
  if (volkInitialize() != VK_SUCCESS)
  {
    return VK_FALSE;
  }

  // Init and test instance extensions
  std::vector<const char*> extensionNames_v;
  extensionNames_v.reserve(3);
  int32_t extIdx = -1;

  // We always create graphics-enabled instance
  extensionNames_v.push_back("VK_KHR_surface");
#if (_WIN32 || _WIN64)
  extensionNames_v.push_back("VK_KHR_win32_surface");
  //#elif
    // TODO:
#endif

#ifdef _DEBUG
  extensionNames_v.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

  if (!TestInstanceExtensions(extensionNames_v, nullptr))
  {
    return VK_FALSE;
  }

  // Init and test instance layers
  std::vector<const char*> layerNames_v;
#ifdef _DEBUG
  layerNames_v.push_back("VK_LAYER_KHRONOS_validation");
#endif

  if (!TestInstanceLayers(layerNames_v))
  {
    return VK_FALSE;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Create instance
  const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;

  VkApplicationInfo applicationInfo = {};
  applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  //applicationInfo.apiVersion = ;
  applicationInfo.applicationVersion = bhSystem::ApplicationVersion();
  applicationInfo.engineVersion = bhSystem::EngineVersion();
  applicationInfo.pApplicationName = bhSystem::ApplicationName();
  applicationInfo.pEngineName = bhSystem::EngineName();
  applicationInfo.apiVersion = VK_MAKE_VERSION(rs.vk.versionMajor, rs.vk.versionMinor, rs.vk.versionPatch);

  VkInstanceCreateInfo instanceCI = {};
  instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCI.pApplicationInfo = &applicationInfo;
  instanceCI.enabledLayerCount = static_cast<uint32_t>(layerNames_v.size());
  instanceCI.ppEnabledLayerNames = layerNames_v.data();
  instanceCI.enabledExtensionCount = static_cast<uint32_t>(extensionNames_v.size());
  instanceCI.ppEnabledExtensionNames = extensionNames_v.data();

  if (vkCreateInstance(&instanceCI, ctxInst.allocator, &(ctxInst.instance)) == VK_SUCCESS)
  {
    volkLoadInstance(ctxInst.instance);
    if (InitExtensionFunctions())
    {
      return InitDebugCallback();
    }
  }
  return VK_FALSE;
}

void bhVKContext::DestroyDebugCallback()
{
  ctxInst.DestroyDebugReportCallbackEXT(ctxInst.instance, ctxInst.debugReportCallback, ctxInst.allocator);
}

void bhVKContext::DestroyInstance()
{
  DestroyDebugCallback();

  vkDestroyInstance(ctxInst.instance, ctxInst.allocator);
  ctxInst.instance = VK_NULL_HANDLE;
}

bool bhVKContext::Init(SDL_Window* wnd)
{
  if (InitInstance() == VK_TRUE)
  {
    rd = new bhVKRenderDevice();
    return rd->Init(wnd, ctxInst.instance, ctxInst.allocator) == VK_TRUE;
  }
  return false;
}

void bhVKContext::Destroy()
{
  delete rd;
  rd = nullptr;

  DestroyInstance();
}

void bhVKContext::BeginFrame()
{
  rd->BeginFrame();
}

void bhVKContext::EndFrame()
{
  rd->EndFrame();
}

bool bhVKContext::UploadWorldMesh(bhWorldPipeline* pl, bhMesh* mesh)
{
  return static_cast<bhVKWorldPipeline*>(pl)->UploadWorldMesh(rd, mesh);
}

bool bhVKContext::InitImGui(SDL_Window* wnd)
{
  return rd->InitImGui(wnd, &(ctxInst.instance));
}

void bhVKContext::DestroyImGui()
{
  rd->DestroyImGui();
}

bhWorldPipeline* bhVKContext::CreateWorldPipeline()
{
  return bhVKWorldPipeline::Create(rd, 1280, 768);
}

void bhVKContext::BindWorldPipeline(const bhWorldPipeline* pl)
{
  bhVKWorldPipeline::Bind(rd, static_cast<const bhVKWorldPipeline*>(pl));
}

bhWorldMaterial* bhVKContext::LoadWorldMaterial(const char* filename)
{
  return bhVKWorldPipeline::LoadMaterial(rd, filename);
}

void bhVKContext::DestroyWorldMaterial(bhWorldMaterial* mat)
{
  bhVKWorldPipeline::DestroyMaterial(rd, static_cast<bhVKWorldPipeline::Material*>(mat));
}

void bhVKContext::BindWorldMaterial(const bhWorldMaterial* mat)
{
  bhVKWorldPipeline::BindMaterial(rd, static_cast<const bhVKWorldPipeline::Material*>(mat));
}

void bhVKContext::SetWorldCameraView(bhWorldPipeline* pl, const bhCamera::ViewData* vd)
{
  static_cast<const bhVKWorldPipeline*>(pl)->SetWorldCameraView(rd, vd);
}

void bhVKContext::DrawWorldMesh(const bhWorldPipeline* pl, const bhMesh* mesh, const glm::mat4& transform)
{
  static_cast<const bhVKWorldPipeline*>(pl)->DrawWorldMesh(rd, mesh, transform);
}
