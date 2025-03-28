#include <assert.h>
#include "bhWindow.h"
#include "bhConfig.h"

#ifdef _DEBUG
#include "bhLog.h"
#endif //_DEBUG

bhWindow* bhCreateWindow(const char* windowTitle)
{
	int numDisplays = SDL_GetNumVideoDisplays();
	if (numDisplays < 0)
	{
	#ifdef  _DEBUG
		bhLog_Message(LOG_TYPE_ERROR, "SDL Error enumerating displays: SDL_GetNumVideoDisplays returned %d", numDisplays);
	#endif //  _DEBUG
		return NULL;
	}

	const bhWindowSettings* ws = bhConfig_GetWindowSettings();
	int preferredDisplay = ws->monitor_index < numDisplays ? ws->monitor_index : numDisplays;

	int width = 0, height = 0;
	if (bhConfig_UseDesktopMode(ws))
	{
		SDL_DisplayMode displayMode;
		if (SDL_GetDesktopDisplayMode(preferredDisplay, &displayMode) == 0)
		{
			width = displayMode.w;
			height = displayMode.h;
		}
	}
	else
	{
		width = ws->width;
		height = ws->height;
	}
	assert((width > 0) && (height > 0));

	Uint32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;
	if (bhConfig_UseFullscreen(ws))
	{
		flags |= SDL_WINDOW_FULLSCREEN; // TODO: Is this compatible with VK fullscreen surface?
	}
	const bhRenderSettings* rs = bhConfig_GetRenderSettings();

	bhWindow* newWindow = SDL_CreateWindow(
		windowTitle,
		SDL_WINDOWPOS_CENTERED_DISPLAY(preferredDisplay), SDL_WINDOWPOS_CENTERED_DISPLAY(preferredDisplay),
		width, height,
		flags);

	return newWindow;
}

void bhDestroyWindow(bhWindow* wnd)
{
	SDL_DestroyWindow(wnd);
}

void bhShowWindow(bhWindow* wnd)
{
	assert(wnd != NULL);
	SDL_ShowWindow(wnd);
}

void bhHideWindow(bhWindow* wnd)
{
	assert(wnd != NULL);
	SDL_HideWindow(wnd);
}

void bhGetWindowSize(bhWindow* wnd, int* w, int* h)
{
	assert(wnd != NULL);
	SDL_GetWindowSize(wnd, w, h);
}

float bhGetWindowAspect(bhWindow* wnd)
{
	assert(wnd != NULL);
	int w = 0, h = 0;
	SDL_GetWindowSize(wnd, &w, &h);
	if ((w > 0) && (h > 0))
	{
		return (float)w / (float)h;
	}
	return 0.f;
}
