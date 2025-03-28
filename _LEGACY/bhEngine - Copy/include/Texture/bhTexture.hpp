#ifndef BH_TEXTURE_HPP
#define BH_TEXTURE_HPP

#include <inttypes.h>
#include "bhTypes.hpp"

class bhTexture
{
public:
  virtual bool IsValid() = 0;

protected:
  bhTexture() = default;

  bhSize3D_<uint32_t> extent;

private:
};

#endif //BH_TEXTURE_HPP
