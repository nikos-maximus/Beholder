#include "SDL3/SDL_assert.h"
#include "bhmap.hpp"

bhMap::bhMap(uint8_t _xsiz, uint8_t _ysiz)
  :xsiz(_xsiz), ysiz(_ysiz)
{
  blocks = new Block[xsiz * ysiz];
}

bhMap::~bhMap()
{
  delete[] blocks;
}

const bhMap::Block* bhMap::GetBlock(uint8_t x, uint8_t y) const
{
  SDL_assert((x < xsiz) && (y < ysiz));
  return &(blocks[y * xsiz + x]);
}
