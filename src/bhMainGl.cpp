#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <GL/gl3w.h>
#include "bhConfig.hpp"
#include "bhPlatform.hpp"
#include "bhGl.hpp"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		bhPlatform::SetDataDir(argv[1]);
	}

	if (SDL_Init(SDL_INIT_VIDEO))
	{
		bhConfig cfg;
		//bhConfig::Load()
		SDL_PropertiesID props = bhConfig::CreateProperties(cfg);
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);

		SDL_Window* mainWindow = SDL_CreateWindowWithProperties(props);
		if (mainWindow)
		{
			SDL_GLContext glCtx = SDL_GL_CreateContext(mainWindow);
			if (glCtx)
			{
				if (!SDL_GL_SetSwapInterval(cfg.window.use_vsync ? 1 : 0))
				{
					// TODO: Report SDL error
				}
				if (bhGl::Init())
				{
					SDL_ShowWindow(mainWindow); // TODO: Error check
					bool running = true;
					while (running)
					{
						SDL_Event evt;
						while (SDL_PollEvent(&evt))
						{
							switch (evt.type)
							{
								case SDL_EVENT_QUIT:
								{
									running = false;
									break;
								}
							}
						}

						bhGl::BeginFrame();
						bhGl::EndFrame();
						SDL_GL_SwapWindow(mainWindow);
					}
				}
				SDL_GL_DestroyContext(glCtx);
			}
			SDL_DestroyWindow(mainWindow);
		}
		SDL_Quit();
		return 0;
	}
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
	return -1;
}
