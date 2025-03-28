#include <SDL_vulkan.h>
#include "bhSystem.hpp"
#include "bhConfig.h"
#include "Platform/bhPlatform.hpp"
#include "bhLog.h"
#include "bhUtil.hpp"
#include "Mesh/bhMeshCache.hpp"
#include "Texture/bhTextureCache.hpp"

#define ENGINE_VER_MAJOR 1
#define ENGINE_VER_MINOR 1
#define ENGINE_VER_REV 1

////////////////////////////////////////////////////////////////////////////////
static bhSystem g_system;

void ErrorFunc_GLFW(int error_code, const char* description)
{
	bhLog_Message(bhLogPriority::LP_ERROR, description);
}

bhSystem& bhSystem::Get()
{
	return g_system;
}

SDL_Window* bhSystem::MainWindow()
{
	SDL_assert(mainWnd != nullptr);
	return mainWnd;
}

bhRenderDevice* bhSystem::RenderDevice()
{
	return renderDevice.get();
}

bool bhSystem::CreateMainWindow()
{
	int numDisplays = SDL_GetNumVideoDisplays();
	if (numDisplays < 0)
	{
	#ifdef  _DEBUG
		bhLog_Message(LP_ERROR, "SDL Error enumerating displays: SDL_GetNumVideoDisplays returned %d", numDisplays);
	#endif //  _DEBUG
		return NULL;
	}

	const bhWindowSettings* ws = bhConfig_GetWindowSettings();
	int preferredDisplay = bhUtil::Min(ws->display_index, numDisplays);

	int width = 0, height = 0;
	if (bhConfig_UseDesktopMode(ws))
	{
		SDL_DisplayMode dm;
		if(SDL_GetDesktopDisplayMode(preferredDisplay, &dm))
		{
			width = dm.w;
			height = dm.h;
		}
	}
	else
	{
		width = ws->width;
		height = ws->height;
	}
	SDL_assert((width > 0) && (height > 0));

	Uint32 flags = BH_SDL_API_FLAG | SDL_WINDOW_HIDDEN;
	if (bhConfig_UseFullscreen(ws))
	{
		flags |= SDL_WINDOW_FULLSCREEN; // TODO: Is this compatible with VK fullscreen surface?
	}

#ifdef BH_GPU_API_OPENGL
	renderDevice->InitSDL_GL();
#endif

	mainWnd = SDL_CreateWindow(BH_APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	return mainWnd != nullptr;
}

bool bhSystem::Init(char* argv[])
{
	bhPlatform::Init();
	const char* cfp = bhPlatform::CreateConfigFilePath("Config.cfg");
	if (bhConfig_Load(cfp)) // 0 is success
	{
		bhLog_Message(LP_WARN, "Could not load config file, using default values");
		bhConfig_InitGlobal();
	}
	bhPlatform::FreePath(cfp);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		return false;
	}
	SDL_version version;
	SDL_GetVersion(&version);
	bhLog_Message(LP_INFO, "SDL version %d.%d.%d", version.major, version.minor, version.patch);
	if (!CreateMainWindow())
	{
		return false;
	}

#ifdef BH_GPU_API_VULKAN
	if (vkInst.Init() != VK_TRUE)
	{
		return false;
	}
	renderDevice.reset(new bhVk::RenderDevice(vkInst));
	VkSurfaceKHR wndSurf = VK_NULL_HANDLE;
	SDL_Vulkan_CreateSurface(mainWnd, vkInst.GetInstance(), &wndSurf);
	if (!renderDevice->Init(wndSurf))
	{
		return false;
	}
#elif BH_GPU_API_OPENGL
	renderDevice.reset(new bhGl::RenderDevice(mainWnd));
	if (!renderDevice->Init())
	{
		return false;
	}
#endif

	int ww, wh;
	SDL_GetWindowSizeInPixels(mainWnd, &ww, &wh);
	if (!renderDevice->CreateWorldPipeline(ww, wh))
	{
		return false;
	}

	bhMeshCache::Get()->InitPathContents();
	bhTextureCache::Get()->InitPathContents();

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

	//	renderDevice.reset(new bhRenderDevice(*instance));
	//	if (!renderDevice->Init())
	//	{
	//		return false;
	//	}

	SDL_ShowWindow(mainWnd);
	return true;

	bhLog_Message(LP_CRITICAL, "Could not initialize SDL: %s", SDL_GetError());
	return false;
}

void bhSystem::Destroy()
{
	//bhAudio::Destroy();
	//bhThreads::Destroy();

	bhTextureCache::Destroy(
		[this](DeviceTexture& tx) {
			if (tx.IsValid())
			{
				renderDevice->DestroyTexture(tx);
			}
		}
	);
	bhMeshCache::Destroy();

	renderDevice->DestroyWorldPipeline();
	renderDevice->Destroy();

#ifdef BH_GPU_API_VULKAN
	vkInst.Destroy();
#endif
	
	SDL_DestroyWindow(mainWnd);
	mainWnd = nullptr;
	SDL_Quit();
	
	bhPlatform::Destroy();
}

const char* bhSystem::EngineName() const
{
	return "BeholderEngine";
}

uint32_t bhSystem::EngineVersion() const
{
	return ((ENGINE_VER_MAJOR) << 22) | ((ENGINE_VER_MINOR) << 12) | (ENGINE_VER_REV);
}

uint32_t bhSystem::ApplicationVersion() const
{
	return 1;
}

float bhSystem::WindowASpect() const
{
	int ww, wh;
	SDL_GetWindowSizeInPixels(mainWnd, &ww, &wh);
	return float(ww) / float(wh);
}
