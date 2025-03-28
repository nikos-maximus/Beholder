#pragma once
//#include "Math/bhMath.h"
#include "Math/bhVector.h"

template<typename Vec2_T, typename Scalar_T>
class bhRect
{
public:
    bhRect(Scalar_T _size)
    {
        extent.x = extent.y = _size / Scalar_T(2);
    }

    bhRect(Vec2_T const& _size)
        : extent(_size* Scalar_T(0.5))
    {}

    inline Vec2_T const& GetPosition() const 
    { 
        return center; 
    }

    inline void SetPosition(Vec2_T const& _pos) { center = _pos; }
    inline void SetPosition(Scalar_T x, Scalar_T y) { center.x = x; center.y = y; }

    inline Vec2_T const& GetExtent() const 
    { 
        return extent; 
    }

    inline Vec2_T GetSize() const 
    { 
        return extent + extent; 
    }

    inline void SetSize(Vec2_T const& siz) { extent = siz * Scalar_T(0.5); }
    inline void SetSize(Scalar_T sizx, Scalar_T sizy, Scalar_T sizz) { SetSize(Vec2_T(sizx, sizy, sizz)); }

    inline bool Contains(Vec2_T const& point) const
    {
        return
            (point.x >= (center.x - extent.x)) && (point.x <= (center.x + extent.x)) &&
            (point.y >= (center.y - extent.y)) && (point.y <= (center.y + extent.y));
    }

    inline bool Contains(bhRect const& box) const
    {
        return Contains(box.center - box.extent) && Contains(box.center + box.extent);
    }

    inline bool Overlaps(bhRect const& box) const
    {
        return !DoesNotOverlap(box);
    }

    inline bool DoesNotOverlap(bhRect const& box) const
    {
        return
            (center.x - extent.x) > (box.center.x + box.extent.x) ||
            (center.x + extent.x) < (box.center.x - box.extent.x) ||

            (center.y - extent.y) > (box.center.y + box.extent.y) ||
            (center.y + extent.y) < (box.center.y - box.extent.y);
    }

protected:
    Vec2_T center;
    Vec2_T extent;

private:
};

typedef bhRect<bhVec2<int>, int> bhRecti;
typedef bhRect<bhVec2f, float> bhRectf;
