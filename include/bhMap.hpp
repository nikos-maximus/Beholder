#ifndef BH_WORLD_HPP
#define BH_WORLD_HPP

#include <inttypes.h>
#include "bhDefines.hpp"
//#include "bhMapRenderer.hpp"

class bhMap
{
public:
  ////////////////////////////////////////
  class Block
  {
  public:
    bool Solid() const { return !(flags & BH_BIT(0)); } //Solid if rightmost bit is 0
    void ToggleSolid() { flags & BH_BIT(0) ? (flags &= ~BH_BIT(0)) : (flags |= BH_BIT(0)); }

  protected:
  private:
    uint8_t flags{ 0 };
  };

  bhMap() = delete;
  bhMap(uint8_t _xsiz, uint8_t _ysiz);
  ~bhMap();

  void GetDims(uint8_t& x, uint8_t& y) const { x = xsiz; y = ysiz; }
  const Block* GetBlock(uint8_t x, uint8_t y) const;
  Block* GetBlock(uint8_t x, uint8_t y);

protected:
private:
  Block* blocks{ nullptr };
  uint8_t xsiz{ 0 }, ysiz{ 0 };
};

#endif //BH_WORLD_HPP
