#include <glm/ext.hpp>
#include "Game/bhWorld.hpp"
#include "bhSystem.hpp"
#include "Mesh/bhMeshCache.hpp"
#include "bhInput.hpp"
#include "bhConfig.h"

//#define BH_MESH_NAME_LEN 64

glm::mat4 bhWorld::identity = glm::mat4(1.0f);
glm::mat4 bhWorld::MapBlock::rotations[BH_NUM_GRID_DIRECTIONS];

bhVk::WorldPipeline::Material g_worldMat;

////////////////////////////////////////////////////////////////////////////////
//inline static void PlayerMove_Hold_CB(bhCommandID cmd, void* wrldPtr)
//{
//    bhWorld* wrld = static_cast<bhWorld*>(wrldPtr);
//    wrld->PlayerMove_Hold(cmd);
//}

//inline static void PlayerTurn_Hold_CB(bhCommandID cmd, void* wrldPtr)
//{
//    bhWorld* wrld = static_cast<bhWorld*>(wrldPtr);
//    wrld->PlayerTurn_Hold(cmd);
//}

void bhWorld::MapBlock::SetupRotations()
{
    rotations[BH_GRID_EAST] = glm::rotate(identity, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotations[BH_GRID_NORTH] = glm::rotate(identity, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    rotations[BH_GRID_WEST] = glm::rotate(identity, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotations[BH_GRID_SOUTH] = glm::rotate(identity, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    rotations[BH_GRID_BOTTOM] = glm::rotate(identity, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    rotations[BH_GRID_TOP] = identity;
}

bhWorld::bhWorld()
    : xForm(glm::identity<glm::mat4x4>())
{
    MapBlock::SetupRotations();
}

////////////////////////////////////////////////////////////////////////////////
//bool bhWorld::SaveBlock(FILE* file, const MapBlock& blk, const bhResourceManager<bhMesh>* worldMeshManager)
//{
//    size_t numMeshes = blk.staticMeshes.size();
//    fwrite(&numMeshes, sizeof(numMeshes), 1, file);
//    for (auto& sm : blk.staticMeshes)
//    {
//        size_t written = 0;
//        written = fwrite(&(sm.transform), sizeof(sm.transform), 1, file);
//        assert(written == 1);
//
//        written = fwrite(&(sm.meshIdx), sizeof(sm.meshIdx), 1, file);
//        assert(written == 1);
//
//        written = fwrite(&(sm.materialIdx), sizeof(sm.materialIdx), 1, file);
//        assert(written == 1);
//    }
//    fwrite(&(blk.flags), sizeof(blk.flags), 1, file);
//    return true;
//}

//bool bhWorld::LoadBlock(FILE* file, MapBlock& blk)
//{
//    size_t numMeshes = 0;
//    fread(&numMeshes, sizeof(numMeshes), 1, file);
//    assert(numMeshes > 0);
//    blk.staticMeshes.resize(numMeshes);
//    for (auto& sm : blk.staticMeshes)
//    {
//        size_t read = 0;
//        read = fread(&(sm.transform), sizeof(sm.transform), 1, file);
//        assert(read == 1);
//
//        read = fread(&(sm.meshIdx), sizeof(sm.meshIdx), 1, file);
//        assert(read == 1);
//
//        read = fread(&(sm.materialIdx), sizeof(sm.materialIdx), 1, file);
//        assert(read == 1);
//    }
//    fread(&(blk.flags), sizeof(blk.flags), 1, file);
//    return true;
//}

//void bhWorld::RegisterCommandHandlers()
//{
//    //bhInput_RegisterCommandHandler(CMD_FWD, nullptr, nullptr, PlayerMove_Hold_CB, this);
//    //bhInput_RegisterCommandHandler(CMD_BACK, nullptr, nullptr, PlayerMove_Hold_CB, this);
//    //bhInput_RegisterCommandHandler(CMD_LTURN, nullptr, nullptr, PlayerTurn_Hold_CB, this);
//    //bhInput_RegisterCommandHandler(CMD_RTURN, nullptr, nullptr, PlayerTurn_Hold_CB, this);
//    //bhInput_RegisterCommandHandler(CMD_LSTRAFE, nullptr, nullptr, PlayerMove_Hold_CB, this);
//    //bhInput_RegisterCommandHandler(CMD_RSTRAFE, nullptr, nullptr, PlayerMove_Hold_CB, this);
//}

//void bhWorld::CreateResources()
//{
//    bhGPUDevice* gpuDevice = bhGPUDevice::Get();
//
//    worldPipeline = gpuDevice->CreatePipeline_World();
//    assert(gpuDevice->IsPipelineValid_World(worldPipeline));
//
//    // Wall
//    {
//        bhWorldMaterialCreateInfo wallMCI = { "wall.png", "wall_normal_specular.png" };
//        materialIDs[0] = gpuDevice->CreateMaterial_World(worldPipeline, &wallMCI);
//
//        bhMesh* wallMesh = meshManager.GetResourceObject(meshIDs[0]);
//        if (wallMesh == nullptr)
//        {
//            int wallMasks = MESH_BINDING_BIT_POSITIONS | MESH_BINDING_BIT_NORMALS | MESH_BINDING_BIT_UV_0 | MESH_BINDING_BIT_TANGENTS;
//            wallMesh = bhMesh_CreateBoxNoTopBottom(0.2f, blockSize.side, blockSize.height, wallMasks);
//            meshIDs[0] = meshManager.AddNamedResource("mesh_wall", wallMesh);
//            //wallMesh->hostMesh = bhMesh_CreatePlane(blockSizeVec.x, blockSizeVec.y, wallMasks);
//            //wallMesh->hostMesh = bhMesh_Load("Cube.001..bhm");
//            gpuDevice->UploadMesh_World(worldPipeline, meshIDs[0]);
//        }
//    }
//
//    // Floor / Ceiling
//    {
//        bhWorldMaterialCreateInfo floorMCI = { "BRICK1.png", "BRICK1_normal_specular.png" };
//        materialIDs[1] = gpuDevice->CreateMaterial_World(worldPipeline, &floorMCI);
//
//        bhMesh* floorMesh = meshManager.GetResourceObject(meshIDs[1]);
//        if (floorMesh == nullptr)
//        {
//            int floorMasks = MESH_BINDING_BIT_POSITIONS | MESH_BINDING_BIT_NORMALS | MESH_BINDING_BIT_UV_0 | MESH_BINDING_BIT_TANGENTS;
//            floorMesh = bhMesh_CreatePlane(blockSize.side, blockSize.side, floorMasks);
//            meshIDs[1] = meshManager.AddNamedResource("mesh_floor", floorMesh);
//            gpuDevice->UploadMesh_World(worldPipeline, meshIDs[1]);
//        }
//    }
//}

//void bhWorld::DestroyResources()
//{
//    meshManager.DestroyAll(MeshDeleteFunc);
//    for (uint8_t m = 0; m < MAX_MESHES; ++m)
//    {
//        meshIDs[m] = 0;
//    }
//
//    bhGPUDevice* gpuDevice = bhGPUDevice::Get();
//    for (uint8_t m = 0; m < MAX_MATERIALS; ++m)
//    {
//        gpuDevice->DestroyMaterial_World(materialIDs[m]);
//        materialIDs[m] = 0;
//    }
//
//    gpuDevice->DestroyPipeline_World(worldPipeline);
//}

bool bhWorld::Init()
{
    auto rd = bhSystem::Get().RenderDevice();

    auto meshRsrc_Wall = bhMeshCache::Get()->CreateMesh("Wall_01");
    auto& mesh_Wall = meshRsrc_Wall->GetResource();
    mesh_Wall->Setup().AddBinding(MESH_BINDING_POSITIONS).AddBinding(MESH_BINDING_NORMALS).AddBinding(MESH_BINDING_UV_0).AddBinding(MESH_BINDING_TANGENTS);
    mesh_Wall->CreatePlane(glm::vec2(blockSize.side, blockSize.side));
    rd->UploadWorldMesh(mesh_Wall.get());
    meshRsrc_Wall->SetReady();

    auto meshRsrc_Suzanne = bhMeshCache::Get()->GetMesh("Suzanne.bms");
    auto& mesh_Suzanne = meshRsrc_Suzanne->GetResource();
    rd->UploadWorldMesh(mesh_Suzanne.get());
    meshRsrc_Suzanne->SetReady();
    DEBUG_testMesh = mesh_Suzanne.get();

    g_worldMat = rd->LoadWorldMaterial("Wall_01.xml");

    camera.SetProjection(bhCamera::DEFAULT_FOV_Y_DEGS, bhSystem::Get().WindowASpect(), 0.1f, 1000.0f);
    camera.SetPosition(glm::vec3(-12.0f, -12.0f, 12.0f));
    camera.LookAt(glm::vec3());

    return true;
}

void bhWorld::Destroy()
{
    auto rd = bhSystem::Get().RenderDevice();

    rd->DestroyWorldMaterial(g_worldMat);
    //DestroyResources();
}

void bhWorld::HandleEvent(const bhEvent::Event& evt)
{
    switch (evt.type)
    {
        case bhEvent::Type::EVT_FWD:
        {
            glm::vec3 vd = camera.GetViewData().pos;
            vd += camera.GetFwdVec() * 0.4f;
            camera.SetPosition(vd);
            return;
        }
        case bhEvent::Type::EVT_BACK:
        {
            glm::vec3 vd = camera.GetViewData().pos;
            vd -= camera.GetFwdVec() * 0.4f;
            camera.SetPosition(vd);
            return;
        }
        case bhEvent::Type::EVT_LEFT:
        {
            glm::vec3 vd = camera.GetViewData().pos;
            vd -= camera.GetRightVec() * 0.4f;
            camera.SetPosition(vd);
            return;
        }
        case bhEvent::Type::EVT_RIGHT:
        {
            glm::vec3 vd = camera.GetViewData().pos;
            vd += camera.GetRightVec() * 0.4f;
            camera.SetPosition(vd);
            return;
        }
        case bhEvent::Type::EVT_MOUSE_MOVE:
        {
            const bhInputSettings* is = bhConfig_GetInputSettings();
            float yaw = (float)evt.data.mouseMotion.xrel * is->mouse_sensitivity;
            float pitch = (float)evt.data.mouseMotion.yrel * is->mouse_sensitivity;
            camera.Yaw(-yaw);
            camera.Pitch(-pitch);
            return;
        }
        default:
        {
            return;
        }
    }
}

void bhWorld::Tick(bhTime_t deltaTime)
{}

void bhWorld::Render()
{
    auto rd = bhSystem::Get().RenderDevice();
    rd->BindWorldPipeline();
    rd->SetWorldCameraView(&camera.GetViewData());
    rd->BindWorldMaterial(g_worldMat);
    rd->RenderWorldMesh(DEBUG_testMesh, xForm);

    for (int row = 0; row < MAX_MAP_DIM; ++row)
    {
        for (int col = 0; col < MAX_MAP_DIM; ++col)
        {
            for (int lvl = 0; lvl < 3; ++lvl)
            {
                MapBlock& block = blocks[row][col][lvl];
                for (int m = 0; m < BH_NUM_GRID_DIRECTIONS; ++m)
                {
                    bhMesh* wm = block.wallMeshes[m];
                    if (wm)
                    {
                        glm::mat4 pos = glm::translate(identity, GetWallTranslation(col, row, lvl, (bhGridDirection)m));
                        pos *= MapBlock::rotations[m];
                        rd->RenderWorldMesh(wm, pos);
                    }
                }

            }
        }
    }
}

//void bhWorld::PlayerMove_Hold(bhCommandID cmd)
//{
//    if (player.IsResting())
//    {
//        glm::vec3 currPos = player.GetPosition();
//        glm::vec3 reqPos = currPos;
//        const glm::vec3* camFwd = player.GetCamera().GetFwdVec();
//        const glm::vec3* camRight = player.GetCamera().GetRightVec();
//        switch (cmd)
//        {
//            case CMD_FWD:
//            {
//                reqPos += *camFwd * blockSize.side;
//                break;
//            }
//            case CMD_BACK:
//            {
//                reqPos -= *camFwd * blockSize.side;
//                break;
//            }
//            case CMD_LSTRAFE:
//            {
//                reqPos -= *camRight * blockSize.side;
//                break;
//            }
//            case CMD_RSTRAFE:
//            {
//                reqPos += *camRight * blockSize.side;
//                break;
//            }
//        }
//
//        int reqx = bhMath_Roundf(reqPos.x / blockSize.side);
//        int reqy = bhMath_Roundf(reqPos.y / blockSize.side);
//        if (!IsBlockSolid(reqx, reqy))
//        {
//            player.StartMove(&reqPos);
//            return;
//        }
//        player.StartMove(&currPos);
//    }
//}
//
//void bhWorld::PlayerTurn_Hold(bhCommandID cmd)
//{
//    if (player.IsResting())
//    {
//        player.StartTurn(cmd);
//    }
//}

bool bhWorld::Save(const char* filePath)
{
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, filePath, "w");
    if (!err)
    {
        // TODO: Report error
        return false;
    }
    assert(file); // If the above check passes, this should never fail

    fwrite(&MAX_MAP_DIM, sizeof(MAX_MAP_DIM), 1, file); //x-map dimension
    fwrite(&MAX_MAP_DIM, sizeof(MAX_MAP_DIM), 1, file); //y-map dimension
    fwrite(&blockSize, sizeof(BlockSize), 1, file);

    uint16_t blockCoords = BLOCKS_DELIM;
    for (uint8_t blockRow = 0; blockRow < MAX_MAP_DIM; ++blockRow)
    {
        for (uint8_t blockCol = 0; blockCol < MAX_MAP_DIM; ++blockCol)
        {
            MapBlock& block = blocks[blockRow][blockCol][1];
            if (block.flags)
            {
                blockCoords = (blockRow << 8) | blockCol;
                fwrite(&blockCoords, sizeof(blockCoords), 1, file);
                fwrite(&(block.flags), sizeof(int), 1, file);
                for (int directionIdx = 0; directionIdx < BH_NUM_GRID_DIRECTIONS; ++directionIdx)
                {
                    if (block.flags & BH_BIT(directionIdx))
                    {
                        //assert(wb.wallMeshes[directionIdx]);
                        //bhHash_t wallMeshHash = wb.wallMeshes[directionIdx]->GetMeshHash();
                        //fwrite(file, &wallMeshHash, sizeof(bhHash_t), 1);
                    }
                }
            }
        }
    }
    fwrite(&blockCoords, sizeof(blockCoords), 1, file);

    fclose(file);
    return true;
}

bool bhWorld::Load(const char* filePath)
{
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, filePath, "r");
    if (!err)
    {
        // TODO: Report error
        return false;
    }
    assert(file); // If the above check passes, this should never fail

    int xdim = 0, ydim = 0;
    fread(&xdim, sizeof(xdim), 1, file); //x-map dimension
    fread(&ydim, sizeof(ydim), 1, file); //y-map dimension
    fread(&blockSize, sizeof(BlockSize), 1, file);

    //AllocBlocks(xdim, ydim);

    uint16_t blockCoords = BLOCKS_DELIM;
    fread(&blockCoords, sizeof(blockCoords), 1, file);
    while (blockCoords != BLOCKS_DELIM)
    {
        uint8_t blockRow = uint8_t(blockCoords >> 8), blockCol = uint8_t(blockCoords);
        MapBlock& block = blocks[blockRow][blockCol][1];

        fread(&(block.flags), sizeof(int), 1, file);
        for (int directionIdx = 0; directionIdx < BH_NUM_GRID_DIRECTIONS; ++directionIdx)
        {
            if (block.flags & BH_BIT(directionIdx))
            {
                //wb->translations[directionIdx] = glm::vec3(blockCol * blockSize.side, blockRow * blockSize.side, 0.0f);
                bhHash_t wallMeshHash = 0;
                fread(&wallMeshHash, sizeof(bhHash_t), 1, file);
                //wb.wallMeshes[directionIdx] = bhSystem::Get().MeshCache()->CreateInstance(wallMeshHash);
            }
        }
        fread(&blockCoords, sizeof(blockCoords), 1, file);
    }

    //player.GetCamera().SetProjection(bhCamera::DEFAULT_FOV_Y_DEGS, bhGetMainWindowAspect(), 0.1f, 1000.0f);
    //player.SetPosition(blockSize.side, blockSize.side, blockSize.height / 2.0f);

    fclose(file);
    return true;
}

//bool bhWorld::Load_Resource(const char* fileName)
//{
//    char* path = bhUtil_CreatePath(bhEnv_GetEnvString(ENV_MAPS_PATH), fileName);
//    bool result = Load(path);
//    bhUtil_FreePath(&path);
//    return result;
//}

//bool bhWorld::Load_Path(const char* path)
//{
//    return Load(path);
//}

//void bhWorld::SetupBlock(int x, int y, bool setupNeighbors)
//{
//    if (!AreCoordsValid(x, y))
//    {
//        return;
//    }
//    ClearBlock(x, y);
//
//    bool solidNeighbors[BH_NUM_GRID_DIRECTIONS];
//    solidNeighbors[BH_GRID_EAST] = (x == (_xdim - 1)) || IsBlockSolid(x + 1, y);
//    solidNeighbors[BH_GRID_NORTH] = (y == (_ydim - 1)) || IsBlockSolid(x, y + 1);
//    solidNeighbors[BH_GRID_WEST] = (x == 0) || IsBlockSolid(x - 1, y);
//    solidNeighbors[BH_GRID_SOUTH] = (y == 0) || IsBlockSolid(x, y - 1);
//    solidNeighbors[BH_GRID_CEIL] = true;
//    solidNeighbors[BH_GRID_FLOOR] = true;
//
//    const float halfHeight = blockSize.height * 0.5f;
//    //glm::vec3 scale(1.0f, 1.4f, 1.7f);
//
//    MapBlock& block = blocks[y][x];
//    if (IsBlockSolid(block))
//    {
//        if (!solidNeighbors[BH_GRID_EAST])
//        {
//            bhStaticMesh eastWall;
//            eastWall.meshIdx = 0;
//            eastWall.materialIdx = 0;
//            //bhTransform_SetScale(wall->worldObject.transform, scale);
//            //bhTransform_Rotate(eastWall.transform, bhCamera::WORLD_UP_VEC, bhMath_Deg2Rad(180.0f));
//            bhTransform_SetTranslation(eastWall.transform, blockSize.side * (x + 0.5f), blockSize.side * y, halfHeight);
//            block.staticMeshes.push_back(eastWall);
//        }
//        if (!solidNeighbors[BH_GRID_NORTH])
//        {
//            bhStaticMesh northWall;
//            northWall.meshIdx = 0;
//            northWall.materialIdx = 0;
//            //bhTransform_SetScale(wall->worldObject.transform, scale);
//            bhTransform_Rotate(northWall.transform, bhCamera::WORLD_UP_VEC, bhMath_Deg2Rad(270.0f));
//            bhTransform_SetTranslation(northWall.transform, blockSize.side * x, blockSize.side * (y + 0.5f), halfHeight);
//            block.staticMeshes.push_back(northWall);
//        }
//        if (!solidNeighbors[BH_GRID_WEST])
//        {
//            bhStaticMesh westWall;
//            westWall.meshIdx = 0;
//            westWall.materialIdx = 0;
//            //bhTransform_SetScale(wall->worldObject.transform, scale);
//            bhTransform_Rotate(westWall.transform, bhCamera::WORLD_UP_VEC, bhMath_Deg2Rad(180.0f));
//            bhTransform_SetTranslation(westWall.transform, blockSize.side * (x - 0.5f), blockSize.side * y, halfHeight);
//            block.staticMeshes.push_back(westWall);
//        }
//        if (!solidNeighbors[BH_GRID_SOUTH])
//        {
//            bhStaticMesh southWall;
//            southWall.meshIdx = 0;
//            southWall.materialIdx = 0;
//            //bhTransform_SetScale(wall->worldObject.transform, scale);
//            bhTransform_Rotate(southWall.transform, bhCamera::WORLD_UP_VEC, bhMath_Deg2Rad(90.0f));
//            bhTransform_SetTranslation(southWall.transform, blockSize.side * x, blockSize.side * (y - 0.5f), halfHeight);
//            block.staticMeshes.push_back(southWall);
//        }
//    }
//    else
//    {
//        {
//            bhStaticMesh ceiling;
//            ceiling.meshIdx = 1;
//            ceiling.materialIdx = 1;
//            bhTransform_Rotate(ceiling.transform, glm::vec3(1.0f, 0.0f, 0.0f), bhMath_Deg2Rad(180.0f));
//            bhTransform_SetTranslation(ceiling.transform, blockSize.side * x, blockSize.side * y, blockSize.height);
//            block.staticMeshes.push_back(ceiling);
//        }
//        {
//            bhStaticMesh floor;
//            floor.meshIdx = 1;
//            floor.materialIdx = 1;
//            //bhTransform_Rotate(floor->worldObject.transform, glm::vec3(1.0f, 0.0f, 0.0f), bhMath_Deg2Rad(90.0f));
//            bhTransform_SetTranslation(floor.transform, blockSize.side * x, blockSize.side * y, 0.0f);
//            block.staticMeshes.push_back(floor);
//        }
//    }
//
//    if (setupNeighbors)
//    {
//        SetupBlock(x + 1, y, false);
//        SetupBlock(x, y + 1, false);
//        SetupBlock(x - 1, y, false);
//        SetupBlock(x, y - 1, false);
//    }
//}

//void bhWorld::ClearBlock(int x, int y)
//{
//    MapBlock& block = blocks[y][x];
//    block.staticMeshes.clear();
//}

//void bhWorld::MeshDeleteFunc(struct bhMesh* mesh)
//{
//    bhMesh_Delete(&mesh);
//    delete mesh;
//}

void bhWorld::GetDimensions(int& x, int& y) const
{
    x = MAX_MAP_DIM;
    y = MAX_MAP_DIM;
}

bool bhWorld::IsBlockSolid(int x, int y, int z) const
{
    if (AreCoordsValid(x, y, z))
    {
        return blocks[y][x][z].flags > 0;
    }
    return true; //Let's treat invalid space as solid, i.e. "sth. that the user could never be in"
}

bool bhWorld::SetBlockSolid(int x, int y, bool solid)
{
    constexpr int z = 1;
    if (!AreCoordsValid(x, y, z))
    {
        return false;
    }

    MapBlock& b = blocks[y][x][z];
    b.flags = solid ? 1 : 0;

    UpdateBlock(x, y, z);

    UpdateBlock(x + 1, y, z);
    UpdateBlock(x, y + 1, z);
    UpdateBlock(x - 1, y, z);
    UpdateBlock(x, y - 1, z);
    UpdateBlock(x, y, z - 1);
    UpdateBlock(x, y, z + 1);
    
    return true;
}

glm::vec3 bhWorld::GetWallTranslation(int x, int y, int z, bhGridDirection dir)
{
    float halfSide = blockSize.side * 0.5f;
    float wallheight = z * blockSize.height + blockSize.height * 0.5f;
    switch (dir)
    {
        case BH_GRID_EAST:      return glm::vec3(blockSize.side * (x + 1), blockSize.side * y + halfSide, wallheight);
        case BH_GRID_NORTH:     return glm::vec3(blockSize.side * x + halfSide, blockSize.side * (y + 1), wallheight);
        case BH_GRID_WEST:      return glm::vec3(blockSize.side * x, blockSize.side * y + halfSide, wallheight);
        case BH_GRID_SOUTH:     return glm::vec3(blockSize.side * x + halfSide, blockSize.side * y, wallheight);
        case BH_GRID_BOTTOM:    return glm::vec3(blockSize.side * x + halfSide, blockSize.side * y + halfSide, blockSize.height * z);
        case BH_GRID_TOP:       return glm::vec3(blockSize.side * x + halfSide, blockSize.side * y + halfSide, blockSize.height * (z + 1));
        default:                assert(false); // Spurious direction
    }
    return {}; // Given the default case, we should never land here
}

bool bhWorld::UpdateBlock(int x, int y, int z)
{
    if (!AreCoordsValid(x, y, z))
    {
        return false;
    }

    MapBlock& block = blocks[y][x][z];
    //float halfSide = blockSize.side * 0.5f;
    //float wallheight = z * blockSize.height + blockSize.height * 0.5f;
    //b->translations[BH_GRID_EAST] = GetWallTranslation(x, y, z, BH_GRID_EAST);
    //b->translations[BH_GRID_NORTH] = GetWallTranslation(x, y, z, BH_GRID_NORTH);
    //b->translations[BH_GRID_WEST] = GetWallTranslation(x, y, z, BH_GRID_WEST);
    //b->translations[BH_GRID_SOUTH] = GetWallTranslation(x, y, z, BH_GRID_SOUTH);
    //b->translations[BH_GRID_BOTTOM] = GetWallTranslation(x, y, z, BH_GRID_BOTTOM);
    //b->translations[BH_GRID_TOP] = GetWallTranslation(x, y, z, BH_GRID_TOP);

    //////////////////////////////////////////////////////////////////////////////////
    // East
    if (!IsBlockSolid(x + 1, y, z) && block.flags > 0)
    {
        auto meshRsrc = bhMeshCache::Get()->GetMesh("Wall_01");
        block.wallMeshes[BH_GRID_EAST] = meshRsrc->GetResource().get();
    }
    else
    {
        block.wallMeshes[BH_GRID_EAST] = nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // North
    if (!IsBlockSolid(x, y + 1, z) && block.flags > 0)
    {
        auto meshRsrc = bhMeshCache::Get()->GetMesh("Wall_01");
        block.wallMeshes[BH_GRID_NORTH] = meshRsrc->GetResource().get();
    }
    else
    {
        block.wallMeshes[BH_GRID_NORTH] = nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // West
    if (!IsBlockSolid(x - 1, y, z) && block.flags > 0)
    {
        auto meshRsrc = bhMeshCache::Get()->GetMesh("Wall_01");
        block.wallMeshes[BH_GRID_WEST] = meshRsrc->GetResource().get();
    }
    else
    {
        block.wallMeshes[BH_GRID_WEST] = nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // South
    if (!IsBlockSolid(x, y - 1, z) && block.flags > 0)
    {
        auto meshRsrc = bhMeshCache::Get()->GetMesh("Wall_01");
        block.wallMeshes[BH_GRID_SOUTH] = meshRsrc->GetResource().get();
    }
    else
    {
        block.wallMeshes[BH_GRID_SOUTH] = nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Bottom
    if (!IsBlockSolid(x, y, z - 1) && block.flags > 0)
    {
        auto meshRsrc = bhMeshCache::Get()->GetMesh("Wall_01");
        block.wallMeshes[BH_GRID_BOTTOM] = meshRsrc->GetResource().get();
    }
    else
    {
        block.wallMeshes[BH_GRID_BOTTOM] = nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Top
    if (!IsBlockSolid(x, y, z + 1) && block.flags > 0)
    {
        auto meshRsrc = bhMeshCache::Get()->GetMesh("Wall_01");
        block.wallMeshes[BH_GRID_TOP] = meshRsrc->GetResource().get();
    }
    else
    {
        block.wallMeshes[BH_GRID_TOP] = nullptr;
    }

    return true;
}
