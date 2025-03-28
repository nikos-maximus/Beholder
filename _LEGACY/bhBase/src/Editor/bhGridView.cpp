#include <imgui.h>
#include "Game/bhWorld.hpp"
#include "Editor/bhGridView.hpp"
#include "Texture/bhImage.h"
#include "bhUtil.hpp"

bhGridView::bhGridView(bhWorld& _world)
    : world(_world)
{}

bool bhGridView::Import(const char* filePath) const
{
    return world.Import(filePath);
}

bool bhGridView::CreateFromImage(char const* path)
{
    bhImage* img = bhImage_CreateFromFile(path, 0);
    if (img == nullptr)
    {
        return false;
    }

    if (!New(img->width, img->height))
    {
        return false;
    }

    stbi_uc const* currPixel = nullptr;
    int rowOffset = 0;
    int xDim, yDim;
    world.GetDimensions(xDim, yDim);
    //for (int row = 0; row < MAX_MAP_DIM; ++row)
    //{
    //    for (int col = 0; col < MAX_MAP_DIM; ++col)
    //    {
    //        currPixel = &(img->pixels[(rowOffset + col) * img->numComponents]);
    //        //SetBlockSolid(col, row, *currPixel == 0);
    //    }
    //    rowOffset += MAX_MAP_DIM;
    //}

    //for (int row = 0; row < MAX_MAP_DIM; ++row)
    //{
    //    for (int col = 0; col < MAX_MAP_DIM; ++col)
    //    {
    //        //SetupBlock(col, row, false);
    //    }
    //}
    return true;
}

void bhGridView::SetPlayerPosition(int x, int y)
{
    //if (AreCoordsValid(x, y))
    //{
    //    player.SetPosition(x * blockSize.side, y * blockSize.side, blockSize.height / 2.0f);
    //}
}

bool bhGridView::New(int xdim, int ydim)
{
    //CreateResources();
    //AllocBlocks(xdim, ydim);
    //for (int row = 0; row < ydim; ++row)
    //{
    //    for (int col = 0; col < xdim; ++col)
    //    {
    //        SetBlockSolid(col, row, true);
    //    }
    //}

    //player.GetCamera().SetProjection(bhCamera::DEFAULT_FOV_Y_DEGS, bhGetMainWindowAspect(), 0.1f, 1000.0f);
    //player.SetPosition(blockSize.side, blockSize.side, blockSize.height / 2.0f);
    return true;
}

void bhGridView::Display(bool* show)
{
    if (!*show)
    {
        return;
    }
    if (ImGui::Begin("Map", show))
    {
        static constexpr float MARGIN = 0.0f;
        const ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
        float x = canvasOrigin.x + MARGIN;
        float y = canvasOrigin.y + MARGIN;
        static constexpr float TILE_SZ = 20.0f;
        //float spacing = 4.0f;

        static ImVec4 colf = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        ImU32 col = ImColor(colf);

        int xDim, yDim;
        world.GetDimensions(xDim, yDim);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        for (int by = yDim - 1; by >= 0; --by)
        {
            for (int bx = 0; bx < xDim; ++bx)
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

        //glm::vec3 playerPos = world.GetPlayerGridPosition();
        col = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

        //ImVec2 playerIndPos = ImVec2(
        //    (playerPos.x * TILE_SZ) + canvasOrigin.x + MARGIN,
        //    ((MAX_MAP_DIM - playerPos.y) * TILE_SZ) + canvasOrigin.y + MARGIN);

        //static constexpr float HALF_TILE_SZ = TILE_SZ / 2.0f;
        //static constexpr float IND_HALFSIZ = HALF_TILE_SZ / 2.0f;
        //draw_list->AddRectFilled(
        //    ImVec2(playerIndPos.x + HALF_TILE_SZ - IND_HALFSIZ, playerIndPos.y - HALF_TILE_SZ - IND_HALFSIZ),
        //    ImVec2(playerIndPos.x + HALF_TILE_SZ + IND_HALFSIZ, playerIndPos.y - HALF_TILE_SZ + IND_HALFSIZ),
        //    col, 0.0f, 0);

        ImGuiIO& io = ImGui::GetIO();
        const ImVec2 mouse_pos_in_canvas(io.MousePos.x - canvasOrigin.x, io.MousePos.y - canvasOrigin.y);

        ImVec2 availRegion = ImGui::GetContentRegionAvail();
        if ((availRegion.x > 0.0f) && (availRegion.y > 0.0f))
        {
            ImGui::InvisibleButton("canvas", availRegion, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    int bx = int(mouse_pos_in_canvas.x / TILE_SZ);
                    int by = yDim - (int(mouse_pos_in_canvas.y / TILE_SZ) + 1);
                    world.SetBlockSolid(bx, by, !world.IsBlockSolid(bx, by));
                    //world.SetupBlock(bx, by, true);
                }
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    int bx = int(mouse_pos_in_canvas.x / TILE_SZ);
                    int by = yDim - (int(mouse_pos_in_canvas.y / TILE_SZ) + 1);
                    //world.SetPlayerPosition(bx, by);
                }
            }
        }
    }
    ImGui::End();
}
