#define  SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_log.h>
#include "bhConfig.hpp"
#include "bhPlatform.hpp"
#include "bhVk.hpp"
#include "bhVkPipeline.hpp"
#include "bhWorld.hpp"
#include "bhAppState.hpp"

//DEBUG
#include "bhGltf.hpp"
#include "bhCamera.hpp"
//DEBUG

bhCamera g_camera;
bhVk::PipelineLayout g_plLayout;
bhVk::GraphicsPipeline g_gfxPl;

////////////////////////////////////////////////////////////////////////////////
class bhApp3DState : public bhAppState
{
public:
  bhVk::RenderDevice* renderDev { nullptr };
};

SDL_AppResult SDL_AppInit(void** appState, int argc, char* argv[])
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
      SDL_Window* mainWindow = SDL_CreateWindowWithProperties(props);
      if (mainWindow)
      {
        bhVk::RenderDevice* rd = bhVk::CreateRenderDevice(mainWindow);
        if (rd)
        {
          bhApp3DState* newState(new bhApp3DState());
          newState->props = props;
          newState->mainWindow = mainWindow;
          newState->renderDev = rd;
          *appState = newState;
          
          if (rd->InitImGui(mainWindow))
          {
            newState->SetImguiReady(true);
            newState->SetImguiVisible(true); //DEBUG
          }
    
          // DEBUG
          // const char* cubeFile = bhPlatform::CreateResourcePath(bhPlatform::ResourceType::RT_MESH, "Cube.glb");
          // bhGltf::ImportData iData;
          // bhGltf::ImportFile(cubeFile, iData);
          // iData.meshes;

          // delete[] cubeFile;

          // bhWorld world;
          // int ww, wh;
          // SDL_GetWindowSize(mainWindow, &ww, &wh);
          // bhCamera& cam = world.GetCamera();
          // cam.SetAspect(float(ww) / float(wh));

          g_plLayout = bhVk::CreatePipelineLayout(rd);
          g_gfxPl.Create(rd, g_plLayout);

          // DEBUG

          SDL_ShowWindow(mainWindow); //TODO: Error check
          return SDL_APP_CONTINUE;
        }
      }
    }
  }
  
  return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void* appState, SDL_Event* evt)
{
  bhApp3DState* state = static_cast<bhApp3DState*>(appState);
  bhVk::RenderDevice* rd = state->renderDev;
  if (state->IsImguiVisible())
  {
    rd->ProcessImGuiEvent(evt);
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
  bhApp3DState* state = static_cast<bhApp3DState*>(appState);
  bhVk::RenderDevice* rd = state->renderDev;
  
  rd->BeginFrame(g_gfxPl, &g_camera);
  
  if (state->IsImguiVisible())
  {
    rd->BeginImGuiFrame();
    rd->EndImGuiFrame();
  }
  
  rd->EndFrame();

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appState, SDL_AppResult result)
{
  bhApp3DState* state = static_cast<bhApp3DState*>(appState);
  
  bhVk::RenderDevice* rd = state->renderDev;
  if (rd)
  {
    if (state->IsImguiReady())
    {
      rd->DestroyImGui();
    }
    // DEBUG
    g_gfxPl.Destroy(rd);
    bhVk::DestroyPipelineLayout(rd, g_plLayout);
    // DEBUG

    bhVk::DestroyRenderDevice(rd);
  }

  if (state->mainWindow)
  {
    SDL_DestroyWindow(state->mainWindow);
  }

  bhVk::DestroyInstance();
  bhConfig::DestroyProperties(state->props);
  
  delete state;
  SDL_Quit();

  if (result !=  SDL_APP_SUCCESS)
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
  }
}
