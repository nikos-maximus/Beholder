#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "bhSystem.hpp"
#include "VK/bhInstanceVk.hpp"
#include "VK/bhDeviceVk.hpp"
#include "VK/bhTextureCacheVk.hpp"
#include "bhConfig.hpp"
#include "Platform/bhPlatform.hpp"
#include "bhLog.h"
#include "bhUtil.hpp"
#include "Mesh/bhMeshCache.hpp"

#define ENGINE_VER_MAJOR 1
#define ENGINE_VER_MINOR 1
#define ENGINE_VER_REV 1

namespace bhSystem
{
  static SDL_Window* g_mainWnd{ nullptr };
  static VkSurfaceKHR g_wndSurface{ VK_NULL_HANDLE };

  static bhConfig g_config;

  ////////////////////////////////////////////////////////////////////////////////
  SDL_Window* MainWindow()
  {
    SDL_assert(g_mainWnd);
    return g_mainWnd;
  }

  const bhConfig* Config()
  {
    return &g_config;
  }

  SDL_Window* CreateMainWindow(bhConfig& cfg)
  {
    //int numDisplays = 0;
    //SDL_DisplayID* displays = SDL_GetDisplays(&numDisplays);
    //if (displays == nullptr)
    //{
    //	bhLog_Message(bhLogPriority::LP_CRITICAL, "Error getting SDL displays: %s", SDL_GetError());
    //	return false;
    //}
    //SDL_free(displays);
    //displays = nullptr;

    if (cfg.windowSt.display_index == SDL_MAX_UINT32) //Display index not set
    {
      cfg.windowSt.display_index = SDL_GetPrimaryDisplay();
    }

    if (cfg.windowSt.UseDesktopMode())
    {
      const SDL_DisplayMode* dm = SDL_GetDesktopDisplayMode(cfg.windowSt.display_index);
      if (!dm)
      {
        bhLog_Message(bhLogPriority::LP_CRITICAL, "%s", SDL_GetError()); // TODO: Message phrasing?
        return nullptr;
      }
      cfg.windowSt.w = dm->w;
      cfg.windowSt.h = dm->h;
    }
    SDL_assert((cfg.windowSt.w > 0) && (cfg.windowSt.h > 0));

    Uint32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;
    if (cfg.windowSt.UseFullscreen())
    {
      flags |= SDL_WINDOW_FULLSCREEN; // TODO: Is this compatible with VK fullscreen surface?
    }

    SDL_Window* newWnd = SDL_CreateWindow(BH_APP_NAME, cfg.windowSt.w, cfg.windowSt.h, flags);
    if (newWnd)
    {
      SDL_SetWindowPosition(newWnd, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED); // TODO: Check return
    }
    return newWnd;
  }

  bool Init(char* argv[])
  {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
      return false;
    }
    SDL_Version version;
    SDL_GetVersion(&version);
    bhLog_Message(LP_INFO, "SDL version %d.%d.%d", version.major, version.minor, version.patch);

    bhPlatform::Init();
    const char* cfp = bhPlatform::CreateConfigFilePath("Config.cfg");
    if (bhConfig::Load(cfp, g_config)) // 0 is success
    {
      bhLog_Message(LP_WARN, "Could not load g_config file, using default values");
    }
    bhPlatform::FreePath(cfp);

    if (!bhVk::CreateInstance()) return false;

    g_mainWnd = CreateMainWindow(g_config);
    if (!g_mainWnd) return false;
    if (!SDL_Vulkan_CreateSurface(g_mainWnd, bhVk::GetInstance().instance, bhVk::GetInstance().alloc, &g_wndSurface)) return false;
    if (!bhVk::CreateRenderDevice(bhVk::GetRenderDevice(), bhVk::GetInstance(), g_wndSurface)) return false;

    if (!bhVk::CreateImGui(bhVk::GetRenderDevice(), bhVk::GetInstance(), g_mainWnd)) return false;

    //int ww, wh;
    //SDL_GetWindowSizeInPixels(g_mainWnd, &ww, &wh);
    //if (!g_gpuContext->CreateWorldPipeline(ww, wh))
    //{
    //  return false;
    //}

    //
    //	//bhThreads::bhGame_Init();
    //	//bhThreads::Test();
    //
    //	instance.reset(new bhInstance());
    //	if (!instance->Init())
    //	{
    //		return false;
    //	}
    //	bhImage_Init();

    //	g_gpuContext.reset(new bhRenderDevice(*instance));
    //	if (!g_gpuContext->Init())
    //	{
    //		return false;
    //	}

    SDL_ShowWindow(g_mainWnd);
    return true;

    bhLog_Message(LP_CRITICAL, "Could not initialize SDL: %s", SDL_GetError());
    return false;
  }

  void Destroy()
  {
    bhVk::TextureCache::Destroy();
    bhMeshCache::Destroy();

    //bhAudio::Destroy();
    //bhThreads::Destroy();

    //bhTextureCache::Destroy(
    //	[this](DeviceTexture& tx) {
    //		if (tx.IsValid())
    //		{
    //			g_gpuContext->DestroyTexture(tx);
    //		}
    //	}
    //);

    //g_gpuContext->DestroyWorldPipeline();
    //g_gpuContext->Destroy();
    //delete g_gpuContext;
    //g_gpuContext = nullptr;

    bhVk::DestroyImGui();

    bhVk::DestroyRenderDevice(bhVk::GetRenderDevice(), bhVk::GetInstance());
    bhVk::DestroySurface(bhVk::GetInstance(), g_wndSurface);
    bhVk::DestroyInstance();

    SDL_DestroyWindow(g_mainWnd);
    g_mainWnd = nullptr;
    SDL_Quit();

    bhPlatform::Destroy();
  }

  const char* EngineName()
  {
    return "BeholderEngine";
  }

  uint32_t EngineVersion()
  {
    return ((ENGINE_VER_MAJOR) << 22) | ((ENGINE_VER_MINOR) << 12) | (ENGINE_VER_REV);
  }

  uint32_t ApplicationVersion()
  {
    return 1;
  }

  bool WindowSize(int& w, int& h)
  {
    return SDL_GetWindowSizeInPixels(g_mainWnd, &w, &h) == 0;
  }

  float WindowASpect()
  {
    int ww, wh;
    SDL_GetWindowSizeInPixels(g_mainWnd, &ww, &wh);
    return float(ww) / float(wh);
  }

  void BeginFrame()
  {
    bhVk::BeginFrame(bhVk::GetRenderDevice());
  }

  void EndFrame()
  {
    bhVk::EndFrame(bhVk::GetRenderDevice());
  }

  void BeginImGuiFrame()
  {
    bhVk::BeginImGuiFrame(bhVk::GetRenderDevice());
  }

  void EndImGuiFrame()
  {
    bhVk::EndImGuiFrame(bhVk::GetRenderDevice());
  }

  void SetMouseMode_Cursor()
  {
    SDL_SetWindowMouseGrab(g_mainWnd, SDL_FALSE);
    //SDL_ShowCursor(SDL_ENABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
  }

  void SetMouseMode_Look()
  {
    SDL_SetWindowMouseGrab(g_mainWnd, SDL_TRUE);
    //SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
  }
}
