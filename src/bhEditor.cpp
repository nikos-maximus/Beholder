#include <SDL3/SDL.h> // TODO: Reduce this to necessary
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "bhEditor.hpp"

namespace bhEditor
{
  static int g_running = 1;
  static bool DEBUG_showImGuiDemo = false;

  SDL_PropertiesID CreateProperties()
  {
    SDL_PropertiesID props = SDL_CreateProperties();
    if (props)
    {
      SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
      SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, false);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 450);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 800);
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

  int Run()
  {
    if (SDL_Init(SDL_INIT_VIDEO))
    {
      int window_Width = 800, window_Height = 600;

      SDL_PropertiesID props = CreateProperties();
      if (props)
      {
        SDL_Window* mainWindow = nullptr;
        mainWindow = SDL_CreateWindowWithProperties(props);
        if (mainWindow)
        {
          SDL_Renderer* renderer = SDL_CreateRenderer(mainWindow, nullptr);
          if (renderer)
          {
            ImGuiContext* ctx = ImGui::CreateContext();
            ImGui::SetCurrentContext(ctx);

            ImGui_ImplSDL3_InitForSDLRenderer(mainWindow, renderer);
            ImGui_ImplSDLRenderer3_Init(renderer);
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

              SDL_RenderClear(renderer);

              ImGui_ImplSDL3_NewFrame();
              ImGui_ImplSDLRenderer3_NewFrame();
              ImGui::NewFrame();

              DisplayMenu();

              if (DEBUG_showImGuiDemo) ImGui::ShowDemoWindow(&DEBUG_showImGuiDemo);

              ImGui::Render();
              ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

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

              SDL_RenderPresent(renderer);
            }

            ImGui_ImplSDL3_Shutdown();
            SDL_DestroyRenderer(renderer);
          }
          SDL_DestroyWindow(mainWindow);
        }
        DestroyProperties(props);
      }
    }
    return 0;
  }
} // namespace bhEditor
