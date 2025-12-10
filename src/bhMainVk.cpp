#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include "bhConfig.hpp"
#include "bhPlatform.hpp"
#include "bhVk.hpp"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    bhPlatform::SetDataDir(argv[1]);
  }

  if (SDL_Init(SDL_INIT_VIDEO))
  {
    if (bhVk::CreateInstance())
    {
      bhConfig cfg;
      //bhConfig::Load()
      SDL_PropertiesID props = bhConfig::CreateProperties(cfg);
      SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);

      SDL_Window* mainWindow = SDL_CreateWindowWithProperties(props);
      if (mainWindow)
      {
        if(bhVk::CreateRenderDevice(mainWindow))
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

            bhVk::BeginFrame();
            bhVk::EndFrame();
          }
          bhVk::DestroyRenderDevice();
        }
        SDL_DestroyWindow(mainWindow);
      }
      bhVk::DestroyInstance();
    }
    SDL_Quit();
    return 0;
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
  return -1;
}
