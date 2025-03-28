#ifndef BH_MESH_VK_HPP
#define BH_MESH_VK_HPP

#include "VK/bhTypesVk.hpp"

namespace bhVk
{
  struct Mesh
  {
    Buffer vb;
    Buffer ib;
    uint32_t indexCount{ 0 };
  };
}

#endif //BH_MESH_VK_HPP
