#ifndef BH_VEC2_H
#define BH_VEC2_H

#include "bhMathUtils.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ////////////////////////////////////////////////////////////////////////////////

    typedef struct
    {
        union
        {
            struct
            {
                float x, y;
            };
            struct
            {
                float r, g;
            };
            float v[2];
        };
    }
    bhVec2;

    __forceinline int bhVec2_IsZero(bhVec2 v)
    {
        return ((v.x == 0.f) && (v.y == 0.f)) ? 1 : 0;
    }

    __forceinline bhVec2 bhVec2_Negate(bhVec2 v)
    {
        bhVec2 out = { -v.x, -v.y };
        return out;
    }

    __forceinline bhVec2 bhVec2_Add(bhVec2 v0, bhVec2 v1)
    {
        bhVec2 out = { v0.x + v1.x, v0.y + v1.y };
        return out;
    }

    __forceinline bhVec2 bhVec2_Sub(bhVec2 v0, bhVec2 v1)
    {
        bhVec2 out = { v0.x - v1.x, v0.y - v1.y };
        return out;
    }

    __forceinline bhVec2 bhVec2_ScalarMult(bhVec2 v, float f)
    {
        bhVec2 out = { v.x * f, v.y * f };
        return out;
    }

    __forceinline float bhVec2_Dot(bhVec2 v0, bhVec2 v1)
    {
        return (v0.x * v1.x) + (v0.y * v1.y);
    }

    __forceinline bhVec2 bhVec2_ScalarDiv(bhVec2 v, float f)
    {
        assert(f != 0.f);
        bhVec2 out = { v.x / f, v.y / f };
        return out;
    }

    __forceinline float bhVec2_Length_Sq(bhVec2 v)
    {
        return (v.x * v.x) + (v.y * v.y);
    }

    __forceinline float bhVec2_Length(bhVec2 v)
    {
        return sqrtf(bhVec2_Length_Sq(v));
    }

    __forceinline float bhVec2_Distance_Sq(bhVec2 v0, bhVec2 v1)
    {
        return bhVec2_Length_Sq(bhVec2_Sub(v1, v0));
    }

    __forceinline float bhVec2_Distance(bhVec2 v0, bhVec2 v1)
    {
        return bhVec2_Length(bhVec2_Sub(v1, v0));
    }

    __forceinline bhVec2 bhVec2_Normalize(bhVec2 v)
    {
        if (!bhVec2_IsZero(v))
        {
            return bhVec2_ScalarDiv(v, bhVec2_Length(v));
        }
        return v;
    }

    __forceinline float bhVec2_Angle_Rads(bhVec2 v0, bhVec2 v1)
    {
        if (bhVec2_IsZero(v0) || bhVec2_IsZero(v1))
        {
            return 0.f;
        }
        bhVec2 n0 = bhVec2_Normalize(v0);
        bhVec2 n1 = bhVec2_Normalize(v1);
        return acosf(bhVec2_Dot(n0, n1));
    }

    __forceinline float bhVec2_Angle_Degs(bhVec2 v0, bhVec2 v1)
    {
        return bhMath_Rad2Deg(bhVec2_Angle_Rads(v0, v1));
    }

    __forceinline bhVec2 bhVec2_Project(bhVec2 v, bhVec2 ref)
    {
        ref = bhVec2_Normalize(ref);
        return bhVec2_ScalarMult(ref, bhVec2_Dot(v, ref));
    }

    ////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif //BH_VEC2_H
