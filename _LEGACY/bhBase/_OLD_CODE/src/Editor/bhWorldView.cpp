#include "Editor/bhWorldView.hpp"
#include "Private/bhGPUDevice.hpp"
#include <imgui.h>

bool bhWorldView::Init(int wndWidth, int wndHeight)
{
    bhSize2Di sz{ wndWidth, wndHeight };
    worldViewFB = bhGPUDevice::Get()->CreateFramebuffer(sz, 1, true);
    world.New(16, 16);
    //world.Load_Resource("Map01.bhm");
    //world.CreateFromImage_Resource("DungeonTest_01.png");
    bhInput_RegisterDefaultKeyBindings();
    world.RegisterCommandHandlers();
    return true;
}

void bhWorldView::Destroy()
{
    world.Destroy();
    bhGPUDevice::Get()->DestroyFramebuffer(worldViewFB);
}

void bhWorldView::Display3DView(bool* show, bhTime_t tickTime)
{
    if (!*show)
    {
        return;
    }

    bhGPUDevice* gpuDevice = bhGPUDevice::Get();
    world.Tick(tickTime);
    
    gpuDevice->UseFramebuffer(worldViewFB);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    world.Render();
    gpuDevice->UseWindowFramebuffer();

    if (ImGui::Begin("WorldView", show, ImGuiWindowFlags_NoResize))
    {
        auto img = gpuDevice->GetFramebufferColorAttachment(worldViewFB, 0);
        auto siz = gpuDevice->GetFramebufferSize(worldViewFB);
        ImGui::Image(img, ImVec2(siz.width, siz.height));
        ImGui::End();
    }
}

void bhWorldView::DisplayGrid(bool* show)
{
    if (!*show)
    {
        return;
    }
    if (ImGui::Begin("Map", show))
    {
        static constexpr float MARGIN = 0.f;
        const ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
        float x = canvasOrigin.x + MARGIN;
        float y = canvasOrigin.y + MARGIN;
        static constexpr float TILE_SZ = 20.f;
        //float spacing = 4.f;

        static ImVec4 colf = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        ImU32 col = ImColor(colf);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        int mapXdim, mapYdim;
        world.GetDimensions(&mapXdim, &mapYdim);
        for (int by = mapYdim - 1; by >= 0; --by)
        {
            for (int bx = 0; bx < mapXdim; ++bx)
            {
                if (world.IsBlockSolid(bx, by))
                {
                    draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + TILE_SZ, y + TILE_SZ), col);
                }
                else
                {
                    draw_list->AddRect(ImVec2(x, y), ImVec2(x + TILE_SZ, y + TILE_SZ), col);
                }
                x += TILE_SZ;// +spacing;
            }
            x = canvasOrigin.x + MARGIN;
            y += TILE_SZ;// +spacing;
        }

        glm::vec3 playerPos = world.GetPlayerGridPosition();
        col = ImColor(ImVec4(1.f, 0.f, 0.f, 1.0f));

        ImVec2 playerIndPos = ImVec2(
            (playerPos.x * TILE_SZ) + canvasOrigin.x + MARGIN,
            ((mapYdim - playerPos.y) * TILE_SZ) + canvasOrigin.y + MARGIN);

        static constexpr float HALF_TILE_SZ = TILE_SZ / 2.f;
        static constexpr float IND_HALFSIZ = HALF_TILE_SZ / 2.f;
        draw_list->AddRectFilled(
            ImVec2(playerIndPos.x + HALF_TILE_SZ - IND_HALFSIZ, playerIndPos.y - HALF_TILE_SZ - IND_HALFSIZ),
            ImVec2(playerIndPos.x + HALF_TILE_SZ + IND_HALFSIZ, playerIndPos.y - HALF_TILE_SZ + IND_HALFSIZ),
            col, 0.0f, 0);

        ImGuiIO& io = ImGui::GetIO();
        const ImVec2 mouse_pos_in_canvas(io.MousePos.x - canvasOrigin.x, io.MousePos.y - canvasOrigin.y);

        ImVec2 availRegion = ImGui::GetContentRegionAvail();
        if ((availRegion.x > 0.f) && (availRegion.y > 0.f))
        {
            ImGui::InvisibleButton("canvas", availRegion, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    int bx = int(mouse_pos_in_canvas.x / TILE_SZ);
                    int by = mapYdim - (int(mouse_pos_in_canvas.y / TILE_SZ) + 1);
                    world.SetBlockSolid(bx, by, !world.IsBlockSolid(bx, by));
                    world.SetupBlock(bx, by, true);
                }
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    int bx = int(mouse_pos_in_canvas.x / TILE_SZ);
                    int by = mapYdim - (int(mouse_pos_in_canvas.y / TILE_SZ) + 1);
                    world.SetPlayerPosition(bx, by);
                }
            }
        }
    }
    ImGui::End();
}
