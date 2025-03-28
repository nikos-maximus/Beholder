#pragma once
#include <bhMesh.h>

namespace tinygltf
{
    class Model;
    struct Mesh;
}

namespace bhGltf
{
    ////////////////////////////////////////////////////////////////////////////////
    class bhMesh_gltf : public bhMesh
    {
    public:
        bhMesh_gltf();
        ~bhMesh_gltf();
        bool Import(tinygltf::Model const& model, size_t meshIndex);

    protected:
    private:
    };

#if 0
    enum PrimitiveMode
    {
        POINTS,
        LINES,
        LINE_LOOP,
        LINE_STRIP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,

        PRIMITIVE_MODE_UNKNOWN
    };

    enum ComponentType
    {
        COMPONENT_TYPE_UNKNOWN = 0,
        BYTE = 5120,
        UNSIGNED_BYTE = 5121,
        SHORT = 5122,
        UNSIGNED_SHORT = 5123,
        UNSIGNED_INT = 5125,
        FLOAT = 5126
    };

    enum AccessorType
    {
        // Values represent size of each type in components, see https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#accessor-element-size
        ACCESSOR_TYPE_UNKNOWN = 0,
        SCALAR = 1,
        VEC2 = 2,
        VEC3 = 3,
        VEC4 = 4,
        MAT2 = VEC4,
        MAT3 = 9,
        MAT4 = 16
    };

    template<typename CT> struct  Accessor
    {
        int bufferView = -1;
        int byteOffset = -1;
        int count = 0;
        ComponentType componentType = COMPONENT_TYPE_UNKNOWN;
        AccessorType accessorType = ACCESSOR_TYPE_UNKNOWN;
        std::vector<CT> 
    };
#endif
}
