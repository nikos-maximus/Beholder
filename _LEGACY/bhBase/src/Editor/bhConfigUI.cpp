#include <imgui.h>
#include "Editor/bhConfigUI.hpp"
#include "Platform/bhPlatform.hpp"
#include "bhLog.h"

bhConfigUI::~bhConfigUI()
{
    for (auto& dd : displayData_v)
    {
        for (auto& str : dd.displayModeStrings_v)
        {
            delete[] str;
        }
    }
}

inline void BuildVideoModeString(char** outStr, const SDL_DisplayMode* videoMode, bool isDesktop)
{
    static const int BUF_SZ = 32;
    *outStr = new char[BUF_SZ];
    sprintf_s(*outStr, BUF_SZ, "%d x %d, %d Hz %s", videoMode->w, videoMode->h, videoMode->refresh_rate, isDesktop ? " [current]" : "");
}

bool bhConfigUI::Init()
{
    rs = *bhConfig_GetRenderSettings();
    ws = *bhConfig_GetWindowSettings();

    int numDisplays = SDL_GetNumVideoDisplays();
    if (numDisplays < 1)
    {
        return false;
    }
    displayNames_v.resize(numDisplays);
    displayData_v.resize(numDisplays);

    for (int dispIdx = 0; dispIdx < numDisplays; ++dispIdx)
    {
        displayNames_v[dispIdx] = SDL_GetDisplayName(dispIdx);

        int numDisplayModes = SDL_GetNumDisplayModes(dispIdx);
        DisplayData& currDispData = displayData_v[dispIdx];
        currDispData.displayModes_v.resize(numDisplayModes);
        currDispData.displayModeStrings_v.resize(numDisplayModes);
        for (int modeIdx = 0; modeIdx < numDisplayModes; ++modeIdx)
        {
            SDL_DisplayMode dm;
            SDL_GetDisplayMode(dispIdx, modeIdx, &dm);
            BuildVideoModeString(&(currDispData.displayModeStrings_v[modeIdx]), &dm, false);
        }
    }
    return true;
}

void bhConfigUI::Display(bool* show)
{
    if (!*show)
    {
        return;
    }
    if (ImGui::Begin("Config", show))
    {
        if (ImGui::BeginTabBar("ConfigGroups"))
        {
            if (ImGui::BeginTabItem("Video"))
            {
                ImGui::Combo("Monitor", &(ws.display_index), displayNames_v.data(), int(displayNames_v.size()));

                bool fullcreen = bhConfig_UseFullscreen(&ws) != 0;
                bool useDesktopVideoMode = bhConfig_UseDesktopMode(&ws);
                bool vsync = bhConfig_UseVSync(&ws);

                ImGui::Checkbox("Fullscreen", &fullcreen);
                ImGui::Checkbox("Use desktop video mode", &useDesktopVideoMode);

                DisplayData& currDispData = displayData_v[ws.display_index];

                if (!useDesktopVideoMode)
                {
                    char** modes = currDispData.displayModeStrings_v.data();
                    ImGui::Combo("Video Mode", &currDisplayMode, modes, static_cast<int>(currDispData.displayModes_v.size()));
                }

                const char* anisotropyLevels[] = { "0", "2", "4", "8", "16" };
                ImGui::Combo("Anisotropy", &(rs.anisotropy_level), anisotropyLevels, IM_ARRAYSIZE(anisotropyLevels));

                ImGui::Checkbox("Sync every frame", &vsync);

                if (ImGui::Button("Apply", ImVec2(120, 30)))
                {
                    OnSave();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Audio"))
            {
                bhAudioSettings as = *bhConfig_GetAudioSettings();
                ImGui::SliderFloat("Master gain", &(as.master_gain), 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_None);
                bhConfig_SetAudioSettings(&as);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Input"))
            {
                bhInputSettings is = *bhConfig_GetInputSettings();
                ImGui::SliderFloat("Mouse sensitivity", &(is.mouse_sensitivity), 0.0f, 4.0f, "%.3f", ImGuiSliderFlags_None);
                bhConfig_SetInputSettings(&is);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("System"))
            {
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void bhConfigUI::OnSave()
{
    DisplayData& currDispData = displayData_v[ws.display_index];
    ws.width = currDispData.displayModes_v[currDisplayMode].w;
    ws.height = currDispData.displayModes_v[currDisplayMode].h;
    ws.refresh_rate = currDispData.displayModes_v[currDisplayMode].refresh_rate;
    
    bhConfig_SetWindowSettings(&ws);
    bhConfig_SetRenderSettings(&rs);

    const char* cfp = bhPlatform::CreateConfigFilePath("Config.cfg");
    if (bhConfig_Save(cfp)) // 0 is success
    {
        bhLog_Message(LP_WARN, "Could not load config file, using default values");
    }
    bhPlatform::FreePath(cfp);
}
