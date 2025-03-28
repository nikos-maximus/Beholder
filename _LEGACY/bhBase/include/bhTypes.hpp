#ifndef BH_TYPES_HPP
#define BH_TYPES_HPP

#include "bhDefines.h"

////////////////////////////////////////////////////////////////////////////////
// Pipeline
enum bhShaderStage
{
    SHADER_STG_VERTEX,
    SHADER_STG_TESSELATION_CONTROL,
    SHADER_STG_TESSELATION_EVALUATION,
    SHADER_STG_GEOMETRY,
    SHADER_STG_FRAGMENT,
    SHADER_STG_COMPUTE,

    NUM_SHADER_TYPES,
    SHADER_STG_UNKNOWN = NUM_SHADER_TYPES
};

bhShaderStage bhTypes_GetShaderTypeFromFileExtension(const char* path);

template<typename Scalar_t>
struct bhSize2D_
{
    Scalar_t width{ Scalar_t(0) };
    Scalar_t height{ Scalar_t(0) };
};

typedef bhSize2D_<int> bhSize2Di;

#endif //BH_TYPES_HPP
