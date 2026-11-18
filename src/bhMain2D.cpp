#define  SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_log.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include "bhConfig.hpp"
#include "bhPlatform.hpp"
#include "bhAppState.hpp"

////////////////////////////////////////////////////////////////////////////////
class bhApp2DState : public bhAppState
{
public:
  SDL_Renderer* renderer { nullptr };
};

////////////////////////////////////////////////////////////////////////////////
SDL_AppResult SDL_AppInit(void** appState, int argc, char* argv[])
{
  if (argc > 1)
  {
    bhPlatform::SetDataDir(argv[1]);
  }

  bhConfig cfg;
  //bhConfig::Load()
  SDL_PropertiesID props = bhConfig::CreateProperties(cfg);
  SDL_Window* window = SDL_CreateWindowWithProperties(props);
  
  if (window)
  {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer)
    {
      SDL_SetRenderVSync(renderer, 1);
      SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
      bhApp2DState* state(new bhApp2DState());
      state->props = props;
      state->window = window;
      state->renderer = renderer;
      *appState = state;
      
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      if (ImGui_ImplSDL3_InitForSDLRenderer(window, renderer) && ImGui_ImplSDLRenderer3_Init(renderer))
      {
        state->SetImguiReady(true);
        state->SetImguiVisible(true);
      }
      
      SDL_ShowWindow(window); //TODO: Error check
      
      return SDL_APP_CONTINUE;
    }
  }
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void *appState, SDL_Event *evt)
{
  bhApp2DState* state = static_cast<bhApp2DState*>(appState);
  if (state->IsImguiVisible())
  {
    ImGui_ImplSDL3_ProcessEvent(evt);
  }
  
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
  bhApp2DState* state = static_cast<bhApp2DState*>(appState);
  
  if (state->IsImguiVisible())
  {
    SDL_RenderClear(state->renderer); //TODO: Error check?
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    
    static bool showdemo = true;
    ImGui::ShowDemoWindow(&showdemo);
    
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state->renderer);
    SDL_RenderPresent(state->renderer);
  }
  
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appState, SDL_AppResult result)
{
  bhApp2DState* state = static_cast<bhApp2DState*>(appState);

  if (state->IsImguiReady())
  {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
  
  SDL_DestroyRenderer(state->renderer);
  SDL_DestroyWindow(state->window);
  bhConfig::DestroyProperties(state->props);
  SDL_Quit();
  delete state;
  
  if (result !=  SDL_APP_SUCCESS)
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
  }
}
