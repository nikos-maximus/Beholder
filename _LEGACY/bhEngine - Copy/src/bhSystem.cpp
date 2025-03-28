#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "bhSystem.hpp"
#include "GL/bhGLContext.hpp"
#include "VK/bhVKContext.hpp"
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
  static bhGPUContext* g_gpuContext{ nullptr };
  static bhConfig g_config;
  static bhMeshCache g_meshCache;

  ////////////////////////////////////////////////////////////////////////////////
  void ErrorFunc_GLFW(int error_code, const char* description)
  {
    bhLog_Message(bhLogPriority::LP_ERROR, description);
  }

  SDL_Window* MainWindow()
  {
    SDL_assert(g_mainWnd);
    return g_mainWnd;
  }

  bhGPUContext* GPUContext()
  {
    SDL_assert(g_gpuContext);
    return g_gpuContext;
  }

  bhMeshCache* MeshCache()
  {
    return &g_meshCache;
  }

  const bhConfig* Config()
  {
    return &g_config;
  }

  bool CreateMainWindow()
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

    if (g_config.windowSt.display_index == SDL_MAX_UINT32) //Display index not set
    {
      g_config.windowSt.display_index = SDL_GetPrimaryDisplay();
    }

    if (g_config.windowSt.UseDesktopMode())
    {
      const SDL_DisplayMode* dm = SDL_GetDesktopDisplayMode(g_config.windowSt.display_index);
      if (!dm)
      {
        bhLog_Message(bhLogPriority::LP_CRITICAL, "%s", SDL_GetError()); // TODO: Message phrasing?
        return false;
      }
      g_config.windowSt.w = dm->w;
      g_config.windowSt.h = dm->h;
    }
    SDL_assert((g_config.windowSt.w > 0) && (g_config.windowSt.h > 0));

    Uint32 flags = SDL_WINDOW_HIDDEN;
    if (g_gpuContext)
    {
      flags |= g_gpuContext->GetWindowFlag();
    }
    if (g_config.windowSt.UseFullscreen())
    {
      flags |= SDL_WINDOW_FULLSCREEN; // TODO: Is this compatible with VK fullscreen surface?
    }

    g_gpuContext->PreCreateWindow();
    g_mainWnd = SDL_CreateWindowWithPosition(BH_APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_config.windowSt.w, g_config.windowSt.h, flags);
    return g_mainWnd != nullptr;
  }

  bool Init(char* argv[])
  {
    bhPlatform::Init();
    const char* cfp = bhPlatform::CreateConfigFilePath("Config.cfg");
    if (bhConfig::Load(cfp, g_config)) // 0 is success
    {
      bhLog_Message(LP_WARN, "Could not load g_config file, using default values");
    }
    bhPlatform::FreePath(cfp);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
      return false;
    }
    SDL_version version;
    SDL_GetVersion(&version);
    bhLog_Message(LP_INFO, "SDL version %d.%d.%d", version.major, version.minor, version.patch);

    switch (g_config.renderSt.api)
    {
      case bhGpuApi::BH_OPENGL:
      {
        g_gpuContext = new bhGLContext();
        break;
      }
      case bhGpuApi::BH_VULKAN:
      {
        g_gpuContext = new bhVKContext();
        break;
      }
      case bhGpuApi::BH_NONE:
      default:
      {
        break;
      }
    }
    
    if (g_gpuContext) g_gpuContext->PreCreateWindow();
    if (!CreateMainWindow()) return false;
    if (!g_gpuContext->Init(g_mainWnd)) return false;

    //int ww, wh;
    //SDL_GetWindowSizeInPixels(g_mainWnd, &ww, &wh);
    //if (!g_gpuContext->CreateWorldPipeline(ww, wh))
    //{
    //  return false;
    //}

    g_meshCache.Init();

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
    g_gpuContext->Destroy();
    delete g_gpuContext;
    g_gpuContext = nullptr;

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

  float WindowASpect()
  {
    int ww, wh;
    SDL_GetWindowSizeInPixels(g_mainWnd, &ww, &wh);
    return float(ww) / float(wh);
  }
}
