#ifndef BH_VEC4_H
#define BH_VEC4_H

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
                float x, y, z, w;
            };
            struct
            {
                float r, g, b, a;
            };
            float v[4];
        };
    }
    bhVec4;

    __forceinline int bhVec4_IsZero(bhVec4 v)
    {
        return ((v.x == 0.f) && (v.y == 0.f) && (v.z == 0.f) && (v.w == 0.f)) ? 1 : 0;
    }

    __forceinline bhVec4 bhVec4_Negate(bhVec4 v)
    {
        bhVec4 out = { -v.x, -v.y, -v.z, -v.w };
        return out;
    }

    __forceinline bhVec4 bhVec4_Add(bhVec4 v0, bhVec4 v1)
    {
        bhVec4 out = { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w };
        return out;
    }

    __forceinline bhVec4 bhVec4_Sub(bhVec4 v0, bhVec4 v1)
    {
        bhVec4 out = { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w };
        return out;
    }

    __forceinline bhVec4 bhVec4_ScalarMult(bhVec4 v, float f)
    {
        bhVec4 out = { v.x * f, v.y * f, v.z * f, v.w * f };
        return out;
    }

    __forceinline float bhVec4_Dot(bhVec4 v0, bhVec4 v1)
    {
        return (v0.x * v1.x) + (v0.y * v1.y) + (v0.z * v1.z) + (v0.w * v1.w);
    }

    __forceinline bhVec4 bhVec4_ScalarDiv(bhVec4 v, float f)
    {
        assert(f != 0.f);
        bhVec4 out = { v.x / f, v.y / f, v.z / f };
        return out;
    }

    __forceinline float bhVec4_Length_Sq(bhVec4 v)
    {
        return (v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w);
    }

    __forceinline float bhVec4_Length(bhVec4 v)
    {
        return sqrtf(bhVec4_Length_Sq(v));
    }

    __forceinline float bhVec4_Distance_Sq(bhVec4 v0, bhVec4 v1)
    {
        return bhVec4_Length_Sq(bhVec4_Sub(v1, v0));
    }

    __forceinline float bhVec4_Distance(bhVec4 v0, bhVec4 v1)
    {
        return bhVec4_Length(bhVec4_Sub(v1, v0));
    }

    __forceinline bhVec4 bhVec4_Normalize(bhVec4 v)
    {
        if (bhVec4_IsZero(v))
        {
            return v;
        }
        return bhVec4_ScalarDiv(v, bhVec4_Length(v));
    }

    // Not sure whether those make mathematical sense
    // 
    //__forceinline float bhVec4_Angle_Rads(bhVec4 v0, bhVec4 v1)
    //{
    //    if (bhVec4_IsZero(v0) || bhVec4_IsZero(v1))
    //    {
    //        return 0.f;
    //    }
    //    bhVec4 n0 = bhVec4_Normalize(v0);
    //    bhVec4 n1 = bhVec4_Normalize(v1);
    //    return acosf(bhVec4_Dot(n0, n1));
    //}

    //__forceinline float bhVec4_Angle_Degs(bhVec4 v0, bhVec4 v1)
    //{
    //    return bhMath_Rad2Deg(bhVec4_Angle_Rads(v0, v1));
    //}

    __forceinline bhVec4 bhVec4_Project(bhVec4 v0, bhVec4 v1)
    {
        bhVec4 out = { 0.f, 0.f };
        if (!bhVec4_IsZero(v1))
        {
            out = bhVec4_ScalarMult(v1, bhVec4_Dot(v0, v1) / bhVec4_Length(v1));
        }
        return out;
    }

    __forceinline bhVec4 bhVec2_Project(bhVec4 v, bhVec4 ref)
    {
        ref = bhVec4_Normalize(ref);
        return bhVec4_ScalarMult(ref, bhVec4_Dot(v, ref));
    }

    ////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif //BH_VEC4_H
