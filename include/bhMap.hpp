#ifndef BH_WORLD_HPP
#define BH_WORLD_HPP

#include <inttypes.h>
#include "bhMapRenderer.hpp"

class bhMap
{
public:
  ////////////////////////////////////////
  class Block
  {
  public:
  protected:
  private:
  };

  bhMap() = delete;
  bhMap(uint8_t _xsiz, uint8_t _ysiz);
  ~bhMap();

  const Block* GetBlock(uint8_t x, uint8_t y) const;
  void DrawLayout(bhMapRenderer* renderer);

protected:
private:
  Block* blocks{ nullptr };
  uint8_t xsiz{ 0 }, ysiz{ 0 };
};

#endif //BH_WORLD_HPP
