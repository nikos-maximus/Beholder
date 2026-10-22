#define  SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include "bhConfig.hpp"
#include "bhPlatform.hpp"

////////////////////////////////////////////////////////////////////////////////
struct bhAppState
{
  SDL_PropertiesID props { SDL_MAX_UINT32 };
  SDL_Window* mainWindow { nullptr };
};

SDL_AppResult SDL_AppInit(void** appState, int argc, char* argv[])
{
  if (argc > 1)
  {
    bhPlatform::SetDataDir(argv[1]);
  }

  bhConfig cfg;
  //bhConfig::Load()
  SDL_PropertiesID props = bhConfig::CreateProperties(cfg);
  SDL_Window* mainWindow = SDL_CreateWindowWithProperties(props);
  
  if (mainWindow)
  {
    SDL_ShowWindow(mainWindow); //TODO: Error check
    //bool running = true;
  
    bhAppState* newState(new bhAppState());
    newState->props = props;
    newState->mainWindow = mainWindow;
    *appState = newState;

    return SDL_APP_CONTINUE;
  }
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void *appState, SDL_Event *evt)
{
  switch (evt->type)
  {
    case SDL_EVENT_QUIT:
    {
      return SDL_APP_SUCCESS;
      //break;
    }
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appState)
{
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appState, SDL_AppResult result)
{
  bhAppState* state = static_cast<bhAppState*>(appState);
  
  SDL_DestroyWindow(state->mainWindow);
  bhConfig::DestroyProperties(state->props);
  SDL_Quit();
  delete state;
  
  if (result !=  SDL_APP_SUCCESS)
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
  }
}
