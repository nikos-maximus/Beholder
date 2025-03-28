#ifndef BH_TYPES_HPP
#define BH_TYPES_HPP

//#include "bhDefines.hpp"

using bhTime_t = float;
using bhFloat_t = float;

////////////////////////////////////////////////////////////////////////////////
enum bhShaderStage
{
    SHADER_STG_VERTEX,
    SHADER_STG_TESSELATION_CONTROL,
    SHADER_STG_TESSELATION_EVALUATION,
    SHADER_STG_GEOMETRY,
    SHADER_STG_FRAGMENT,
    SHADER_STG_COMPUTE,

    NUM_SHADER_STAGES,
    SHADER_STG_UNKNOWN = NUM_SHADER_STAGES
};

////////////////////////////////////////////////////////////////////////////////
template<typename Scalar_t>
struct bhSize2D_
{
    Scalar_t width{ Scalar_t(0) };
    Scalar_t height{ Scalar_t(0) };
};

////////////////////////////////////////////////////////////////////////////////
template<typename Scalar_t>
struct bhSize3D_
{
  Scalar_t width{ Scalar_t(0) };
  Scalar_t height{ Scalar_t(0) };
  Scalar_t depth{ Scalar_t(0) };
};

#endif //BH_TYPES_HPP
