#ifndef BH_VEC3_H
#define BH_VEC3_H

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
                float x, y, z;
            };
            struct
            {
                float r, g, b;
            };
            float v[3];
        };
    }
    bhVec3;

    __forceinline int bhVec3_IsZero(bhVec3 v)
    {
        return ((v.x == 0.f) && (v.y == 0.f) && (v.z == 0.f)) ? 1 : 0;
    }
    
    __forceinline bhVec3 bhVec3_Negate(bhVec3 v)
    {
        bhVec3 out = { -v.x, -v.y, -v.z };
        return out;
    }
    
    __forceinline bhVec3 bhVec3_Add(bhVec3 v0, bhVec3 v1)
    {
        bhVec3 out = { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z };
        return out;
    }

    __forceinline bhVec3 bhVec3_Sub(bhVec3 v0, bhVec3 v1)
    {
        bhVec3 out = { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z };
        return out;
    }

    __forceinline bhVec3 bhVec3_ScalarMult(bhVec3 v, float f)
    {
        bhVec3 out = { v.x * f, v.y * f, v.z * f };
        return out;
    }

    __forceinline float bhVec3_Dot(bhVec3 v0, bhVec3 v1)
    {
        return (v0.x * v1.x) + (v0.y * v1.y) + (v0.z * v1.z);
    }

    __forceinline bhVec3 bhVec3_Cross(bhVec3 v0, bhVec3 v1)
    {
        bhVec3 out =
        {
            (v0.y * v1.z) - (v1.y * v0.z),
            (v1.x * v0.z) - (v0.x * v1.z),
            (v0.x * v1.y) - (v1.x * v0.y)
        };
        return out;
    }

    __forceinline bhVec3 bhVec3_ScalarDiv(bhVec3 v, float f)
    {
        assert(f != 0.f);
        bhVec3 out = { v.x / f, v.y / f, v.z / f };
        return out;
    }

    __forceinline float bhVec3_Length_Sq(bhVec3 v)
    {
        return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
    }

    __forceinline float bhVec3_Length(bhVec3 v)
    {
        return sqrtf(bhVec3_Length_Sq(v));
    }

    __forceinline float bhVec3_Distance_Sq(bhVec3 v0, bhVec3 v1)
    {
        return bhVec3_Length_Sq(bhVec3_Sub(v1, v0));
    }

    __forceinline float bhVec3_Distance(bhVec3 v0, bhVec3 v1)
    {
        return bhVec3_Length(bhVec3_Sub(v1, v0));
    }

    __forceinline bhVec3 bhVec3_Normalize(bhVec3 v)
    {
        if (bhVec3_IsZero(v))
        {
            return v;
        }
        return bhVec3_ScalarDiv(v, bhVec3_Length(v));
    }

    __forceinline float bhVec3_Angle_Rads(bhVec3 v0, bhVec3 v1)
    {
        if (bhVec3_IsZero(v0) || bhVec3_IsZero(v1))
        {
            return 0.f;
        }
        bhVec3 n0 = bhVec3_Normalize(v0);
        bhVec3 n1 = bhVec3_Normalize(v1);
        return acosf(bhVec3_Dot(n0, n1));
    }

    __forceinline float bhVec3_Angle_Degs(bhVec3 v0, bhVec3 v1)
    {
        return bhMath_Rad2Deg(bhVec3_Angle_Rads(v0, v1));
    }

    __forceinline bhVec3 bhVec3_Project(bhVec3 v0, bhVec3 v1)
    {
        bhVec3 out = { 0.f, 0.f };
        if (!bhVec3_IsZero(v1))
        {
            out = bhVec3_ScalarMult(v1, bhVec3_Dot(v0, v1) / bhVec3_Length(v1));
        }
        return out;
    }

    __forceinline bhVec3 bhVec2_Project(bhVec3 v, bhVec3 ref)
    {
        ref = bhVec3_Normalize(ref);
        return bhVec3_ScalarMult(ref, bhVec3_Dot(v, ref));
    }

    ////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif //BH_VEC3_H
