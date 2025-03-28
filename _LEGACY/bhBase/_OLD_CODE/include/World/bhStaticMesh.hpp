#ifndef BH_STATIC_MESH_HPP
#define BH_STATIC_MESH_HPP

class bhMeshInstance;
class bhWorldMaterial;

struct bhStaticMesh
{
    bhMeshInstance* mesh{ nullptr };
    bhWorldMaterial* material{ nullptr };
};

#endif //BH_STATIC_MESH_HPP
