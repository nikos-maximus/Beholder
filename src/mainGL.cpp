#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include "bhConfig.hpp"

#include <GL/gl3w.h>

SDL_Window* g_mainWnd = nullptr;

int main(int argc, char* argv[])
{
  bool running = true;

  if (SDL_Init(SDL_INIT_VIDEO))
  {
    bhConfig cfg;
    //bhConfig::Load()
    SDL_PropertiesID props = bhConfig::CreateProperties(cfg);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);

    g_mainWnd = SDL_CreateWindowWithProperties(props);
    if (g_mainWnd)
    {
      SDL_GLContext glCtx = SDL_GL_CreateContext(g_mainWnd);
      if (glCtx)
      {
        if (gl3wInit() == 0) // Success
        {
          //auto res = gl3wIsSupported(4, 6);
          glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
          SDL_ShowWindow(g_mainWnd); // TODO: Error check
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

            glClear(GL_COLOR_BUFFER_BIT);
            SDL_GL_SwapWindow(g_mainWnd);
          }
        }
        SDL_GL_DestroyContext(glCtx);
      }
      SDL_DestroyWindow(g_mainWnd);
    }
    SDL_Quit();
    return 0;
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
}