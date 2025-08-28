#include <SDL3/SDL.h> // TODO: Reduce this to necessary
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "bhEditor.hpp"
#include "bhMap.hpp"
#include "bhMapRenderer.hpp"

namespace bhEditor
{
  static int g_running{ 1 };
  bhMap* g_map{ nullptr };
  bhMapRenderer* g_mapRenderer{ nullptr };
  static bool DEBUG_showImGuiDemo{ false };

  SDL_PropertiesID CreateProperties()
  {
    SDL_PropertiesID props = SDL_CreateProperties();
    if (props)
    {
      SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
      SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, false);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 720);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 1280);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    }
    return props;
  }

  void DestroyProperties(SDL_PropertiesID props)
  {
    SDL_DestroyProperties(props);
  }

  void OnFileNew()
  {

  }

  void OnFileOpen()
  {

  }

  void OnFileSave()
  {

  }

  void OnFileExitEditor()
  {

  }

  void OnFileExit()
  {
    g_running = 0;
  }

  void DisplayMenu()
  {
    if (ImGui::BeginMainMenuBar())
    {
      // File menu
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("New", nullptr)) { OnFileNew(); }
        if (ImGui::MenuItem("Open", nullptr)) { OnFileOpen(); }
        //if (ImGui::MenuItem("Create from Image", nullptr)) { OnFileCreateFromImage(); }
        if (ImGui::MenuItem("Save", nullptr)) { OnFileSave(); }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit editor")) { OnFileExitEditor(); }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) { OnFileExit(); }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View"))
      {
        //if (ImGui::MenuItem("Show grid", nullptr, &showGridView)) {}
        //if (ImGui::MenuItem("Show player view", nullptr, &showWorld3D)) {}
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Resources"))
      {
        //if (ImGui::MenuItem("Show resource browser", nullptr, &showResourceBrowser)) {}
        //if (ImGui::MenuItem("Import geometries", nullptr)) { OnImportGeometries(); }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Config"))
      {
        //if (ImGui::MenuItem("Show engine config", nullptr, &showConfig)) {}
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("DEBUG"))
      {
        if (ImGui::MenuItem("Show imgui demo window", nullptr, &DEBUG_showImGuiDemo)) {}
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
  }

  void DisplayMap()
  {
    g_mapRenderer->DrawWidget(g_map);
  }

  int Run()
  {
    if (SDL_Init(SDL_INIT_VIDEO))
    {
      SDL_PropertiesID props = CreateProperties();
      if (props)
      {
        SDL_Window* mainWindow = nullptr;
        mainWindow = SDL_CreateWindowWithProperties(props);
        if (mainWindow)
        {
          SDL_Renderer* sdlRenderer = SDL_CreateRenderer(mainWindow, nullptr);
          if (sdlRenderer)
          {
            ImGuiContext* ctx = ImGui::CreateContext();
            ImGui::SetCurrentContext(ctx);

            g_map = new bhMap(16, 16);
            g_mapRenderer = new bhMapRenderer(sdlRenderer);
            g_mapRenderer->Reset(g_map);
            g_mapRenderer->Init(); // TODO: Error check
            

            if (ImGui_ImplSDL3_InitForSDLRenderer(mainWindow, sdlRenderer))
            {
              if (ImGui_ImplSDLRenderer3_Init(sdlRenderer))
              {
                SDL_ShowWindow(mainWindow);

                SDL_Event evt;
                while (g_running)
                {
                  while (SDL_PollEvent(&evt))
                  {
                    ImGui_ImplSDL3_ProcessEvent(&evt);
                    if (evt.type == SDL_EVENT_QUIT)
                    {
                      g_running = 0;
                    }
                  }

                  SDL_RenderClear(sdlRenderer);

                  ImGui_ImplSDL3_NewFrame();
                  ImGui_ImplSDLRenderer3_NewFrame();
                  ImGui::NewFrame();

                  DisplayMenu();
                  DisplayMap();

                  if (DEBUG_showImGuiDemo) ImGui::ShowDemoWindow(&DEBUG_showImGuiDemo);

                  ImGui::Render();
                  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer);

                  //SDL_Surface* wndSurface = SDL_GetWindowSurface(mainWindow);
                  //SDL_assert(wndSurface != NULL);
                  //if (SDL_MUSTLOCK(wndSurface))
                  //{
                  //  SDL_LockSurface(wndSurface);
                  //}
                  //if (SDL_MUSTLOCK(wndSurface))
                  //{
                  //  SDL_UnlockSurface(wndSurface);
                  //}
                  //SDL_UpdateWindowSurface(mainWindow);

                  SDL_RenderPresent(sdlRenderer);
                }

                ImGui_ImplSDL3_Shutdown();
              }
            }

            delete g_mapRenderer;
            delete g_map;
            SDL_DestroyRenderer(sdlRenderer);
          }
          SDL_DestroyWindow(mainWindow);
        }
        DestroyProperties(props);
      }
    }
    return 0;
  }
} // namespace bhEditor
