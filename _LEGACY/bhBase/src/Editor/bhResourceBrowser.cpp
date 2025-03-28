#include <imgui.h>
#include "Editor/bhResourceBrowser.hpp"
#include "bhSystem.hpp"
#include "bhTextureCache.hpp"
#include "Mesh/bhMeshCache.hpp"

static char buf[BH_PATH_BUF_LEN];

void bhResourceBrowser::Init()
{}

void bhResourceBrowser::AddTableEntry(bhHash_t hash, const char* info)
{
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    sprintf_s(buf, BH_PATH_BUF_LEN, "%zd", hash);
    ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));

    ImGui::TableSetColumnIndex(1);
    sprintf_s(buf, BH_PATH_BUF_LEN, "%s", info);
    ImGui::TextUnformatted(buf);
}

void bhResourceBrowser::Display(bool* show)
{
    if (!*show)
    {
        return;
    }
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
    bhSystem& sys = bhSystem::Get();
    if (ImGui::Begin("Resource browser", show))
    {
        if (ImGui::BeginTabBar("Resource Types"))
        {
            if (ImGui::BeginTabItem("Textures"))
            {
                if (ImGui::BeginTable("TableTextures", 2, flags))
                {
                    ImGui::TableSetupColumn("Hash");
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableHeadersRow();

                    bhTextureCache* tc = sys.TextureCache();
                    auto& textures = tc->GetTextures();

                    for (auto& t : textures)
                    {
                        AddTableEntry(t.first, t.second.GetName().c_str());
                    }

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Meshes"))
            {
                if (ImGui::BeginTable("TableTextures", 2, flags))
                {
                    ImGui::TableSetupColumn("Hash");
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableHeadersRow();

                    bhMeshCache* mc = sys.MeshCache();
                    auto& meshes = mc->GetMeshes();

                    for (auto& m : meshes)
                    {
                        AddTableEntry(m.first, m.second.GetName().c_str());
                    }

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Materials"))
            {
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}
