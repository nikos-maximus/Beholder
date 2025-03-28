#include "Editor/bhEditor.hpp"
#include "bhSystem.hpp"
#include "VK/bhVkRenderDevice.hpp"
#include "Platform/bhPlatform.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"

#include "bhEvent.hpp"
#include "Game/bhWorld.hpp"

//TEMP
#include "bhConfig.h"

////////////////////////////////////////////////////////////////////////////////
bhEditor::bhEditor(bhWorld& _world)
{
    configUI.reset(new bhConfigUI());
    configUI->Init();

    gridView.reset(new bhGridView(_world));
}

void bhEditor::HandleSDLEvents()
{
    SDL_PumpEvents();
    static SDL_Event evt;
    while (SDL_PollEvent(&evt))
    {
        ImGui_ImplSDL2_ProcessEvent(&evt);
    }
}

void bhEditor::Render()
{
    auto rd = bhSystem::Get().RenderDevice();

    rd->BeginImGuiFrame();
    DisplayMenu();
    configUI->Display(&showConfig);
    gridView->Display(&showGridView);
    if (showDemo)
    {
        ImGui::ShowDemoWindow(&showDemo);
    }
    rd->EndImGuiFrame();
}

void bhEditor::DisplayMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        // File menu
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", nullptr)) { OnFileNew(); }
            if (ImGui::MenuItem("Open", nullptr)) { OnFileOpen(); }
            if (ImGui::MenuItem("Create from Image", nullptr)) { OnFileCreateFromImage(); }
            if (ImGui::MenuItem("Save", nullptr)) { OnFileSave(); }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit editor")) { OnFileExitEditor(); }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { OnFileExit(); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Show grid", nullptr, &showGridView)) {}
            //if (ImGui::MenuItem("Show player view", nullptr, &showWorld3D)) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Resources"))
        {
            if (ImGui::MenuItem("Show resource browser", nullptr, &showResourceBrowser)) {}
            if (ImGui::MenuItem("Import geometries", nullptr)) { OnImportGeometries(); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Config"))
        {
            if (ImGui::MenuItem("Show engine config", nullptr, &showConfig)) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("DEBUG"))
        {
            if (ImGui::MenuItem("Show imgui demo window", nullptr, &showDemo)) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

////////////////////////////////////////////////////////////////////////////////
// File Menu
void bhEditor::OnFileNew()
{
    //worldView.New();
}

void bhEditor::OnFileOpen()
{
    const char* path = bhPlatform::CreateOpenFileDialog(true);
    if (!path)
    {
        //worldView.Load(path);
    }
}

void bhEditor::OnFileCreateFromImage()
{
    const char* path = bhPlatform::CreateOpenFileDialog(true);
    if (path)
    {
        //worldView.CreateFromImage(path);
    }
}

void bhEditor::OnFileSave()
{
    const char* path = bhPlatform::CreateSaveFileDialog(true);
    if (path)
    {
        //worldView.Save(path);
    }
}

void bhEditor::OnFileExit()
{
    bhEvent::Event evt;
    evt.type = bhEvent::Type::EVT_QUIT;
    bhEvent::PushEvent(evt);
}

void bhEditor::OnFileExitEditor()
{
    bhEvent::Event evt;
    evt.type = bhEvent::Type::EVT_TOGGLE_EDITOR;
    bhEvent::PushEvent(evt);
}

void bhEditor::OnImportGeometries()
{
    const char* path = bhPlatform::CreateOpenFileDialog(true);
    if (path)
    {
        gridView->Import(path);
    }
}
