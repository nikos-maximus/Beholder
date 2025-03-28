#ifndef BH_DEFINES_H
#define BH_DEFINES_H

////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Bit
#define BH_BIT(x) (1 << (x))

// C-Interop
#define BH_C_FALSE 0
#define BH_C_TRUE 1

// Rendering
#define BH_RENDERING_PROFILING 0

//#define BH_BUFFER_MAX_VERTS (1024 * 16)
//#define BH_BUFFER_MAX_INDS (1024 * 16)

//#define BH_MAX_INSTANCES 1024

// Mesh
#define BH_USE_MESH_INDEX_TYPE_UNIT16 0

// Platform
#define BH_PATH_BUF_LEN 128

#define BH_INVALID_RESOURCE nullptr

////////////////////////////////////////////////////////////////////////////////
// Types
typedef float bhTime_t;

#endif //BH_DEFINES_H
