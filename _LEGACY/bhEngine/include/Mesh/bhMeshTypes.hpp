#ifndef BH_MESH_TYPES_HPP
#define BH_MESH_TYPES_HPP

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "bhDefines.h"

#if BH_USE_MESH_INDEX_TYPE_UNIT16
typedef uint16_t bhMeshIdx_t;
#else
typedef uint32_t bhMeshIdx_t;
#endif

////////////////////////////////////////////////////////////////////////////////
struct bhMeshVertex_t
{
  typedef glm::vec3 Position_t;
  typedef glm::vec3 Normal_t;
  typedef glm::vec3 Tangent_t;
  typedef glm::vec2 UV_t;

  Position_t position;
  Normal_t normal;
  Tangent_t tangent;
  UV_t uv_0;
};

enum bhMeshBindings
{
  MESH_BINDING_POSITIONS,
  MESH_BINDING_NORMALS,
  MESH_BINDING_TANGENTS,
  //MESH_BINDING_BITANGENTS,
  MESH_BINDING_UV_0,

  NUM_MESH_BINDINGS
};

//enum bhMeshInstanceBindings
//{
//    INSTANCE_POSITIONS = NUM_BINDINGS,
//
//    NUM_BINDINGS
//};

class bhMeshConfig
{
public:
  bhMeshConfig& AddBinding(bhMeshBindings bnd) { bindingsMask |= BH_BIT(bnd); return *this; }
  bhMeshConfig& RemoveBinding(bhMeshBindings bnd) { bindingsMask &= ~BH_BIT(bnd); return *this; }
  bool HasBinding(bhMeshBindings bnd) const { return bindingsMask & BH_BIT(bnd); }

  void SetMask(int mask) { bindingsMask = mask; }
  int GetMask() const { return bindingsMask; }

protected:
private:
  int bindingsMask{ 0 };
};

struct bhMeshDeviceData
{
  bool IsUploaded() const
  {
    return flags & MD_UPLOADED;
  }

  void SetUploaded()
  {
    flags |= MD_UPLOADED;
  }

  struct Offsets
  {
    int32_t vertexOffset{ 0 };
    uint32_t indexOffset{ 0 };
  }
  offsets;

private:
  enum StatusFlags
  {
    MD_UPLOADED = BH_BIT(0),

    NUM_MESH_STATUS_FLAGS
  };

  uint8_t flags;
};

#endif //BH_MESH_TYPES_HPP
