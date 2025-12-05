#include <SDL3/SDL_init.h>

#include "bhConfig.hpp"
#include "bhPlatform.hpp"
#include "bhVk.hpp"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  // TODO:
  if (argc > 1)
  {
    bhPlatform::SetDataDir(argv[1]);
  }

  if (SDL_Init(SDL_INIT_VIDEO))
  {
    bhConfig cfg;
    //bhConfig::Load()
    SDL_PropertiesID props = bhConfig::CreateProperties(cfg);

    if (bhVk::CreateInstance())
    {
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
  }
  return 0;
  //return bhEditor::Run();
}
