#include "ECS/bhWorldObject.hpp"
#include <string.h>

#define BH_MESH_NAME_LEN 64

bool bhStaticMesh_Save(FILE* file, const bhStaticMesh* staticMesh, const bhResourceManager<std::string, struct bhMesh>* worldMeshManager)
{
    size_t written = 0;
    written = fwrite(&(staticMesh->transform), sizeof(staticMesh->transform), 1, file);
    assert(written == 1);

    const char* meshName = worldMeshManager->GetResourceName(staticMesh->meshID);
    assert(meshName != nullptr);
    assert(strlen(meshName) < BH_MESH_NAME_LEN);
    written = fwrite(meshName, sizeof(char), BH_MESH_NAME_LEN, file);
    assert(written == BH_MESH_NAME_LEN);

    //staticMesh->material;

    return true;
}

bool bhStaticMesh_Load(FILE* file, bhStaticMesh* staticMesh)
{
    size_t read = 0;
    read = fread(&(staticMesh->transform), sizeof(staticMesh->transform), 1, file);
    assert(read == 1);
    
    char meshName[BH_MESH_NAME_LEN];
    read = fread(meshName, sizeof(char), BH_MESH_NAME_LEN, file);
    assert(read == BH_MESH_NAME_LEN);

    //staticMesh->material;

    return true;
}
