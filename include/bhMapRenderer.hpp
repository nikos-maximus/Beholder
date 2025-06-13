#ifndef BH_WORLD_RENDERER_HPP
#define BH_WORLD_RENDERER_HPP

#include <inttypes.h>

static const uint8_t TILE_SZ_PX{ 16 };

class bhMapRenderer
{
public:
  virtual bool Init() = 0;

protected:
private:
};

#endif //BH_WORLD_RENDERER_HPP
