#ifndef BH_GRID_HPP
#define BH_GRID_HPP

#include <assert.h>
#include "Math/bhMathUtils.h"

////////////////////////////////////////////////////////////////////////////////
template<typename Block_t>
class bhGrid
{
public:
    inline bool IsValidBlock(int x, int y) const
    {
        return (x >= 0) && (x < _xdim) && (y >= 0) && (y < _ydim);
    }

    inline Block_t* GetBlock(int x, int y) const
    {
        return IsValidBlock(x, y) ? &(blocks[y * _xdim + x]) : nullptr;
    }

    inline void SetBlock(int x, int y, Block_t const& block)
    {
        blocks[y * _xdim + x] = block;
    }

    inline int XDim() const
    {
        return _xdim;
    }

    inline int YDim() const
    {
        return _ydim;
    }

    Block_t* GetBlocks() const
    {
        return blocks;
    }

protected:
    bhGrid()
        : _xdim(0), _ydim(0)
        , blocks(nullptr)
    {}

    bhGrid(int xdim, int ydim)
        : _xdim(xdim), _ydim(ydim)
    {}

    void Resize(int xdim, int ydim)
    {
        delete[] blocks;
        _xdim = xdim;
        _ydim = ydim;
        blocks = new Block_t[_xdim * _ydim];
    }

    virtual ~bhGrid()
    {
        delete[] blocks;
    }

    Block_t* blocks = nullptr;

private:
    int _xdim = 0, _ydim = 0;
};

#endif //BH_GRID_HPP
