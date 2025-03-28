#ifndef BH_QUAT_H
#define BH_QUAT_H

#include "bhVec3.h"
#include "bhMat4.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ////////////////////////////////////////////////////////////////////////////////

    struct bhQuat
    {
        bhVec3 im;
        float re;
    };

    __forceinline bhQuat bhQuat_Add(bhQuat a, bhQuat b)
    {
        bhQuat out = { bhVec3_Add(a.im, b.im), a.re + b.re };
        return out;
    }

    __forceinline bhQuat bhQuat_ScalarMult(bhQuat q, float f)
    {
        bhQuat out = { bhVec3_ScalarMult(q.im, f), q.re * f };
        return out;
    }

    __forceinline bhQuat bhQuat_QuatMult(bhQuat a, bhQuat b)
    {
        bhQuat out;
        out.im = bhVec3_Add(bhVec3_Add(bhVec3_ScalarMult(a.im, b.re), bhVec3_ScalarMult(b.im, a.re)), bhVec3_Cross(a.im, b.im));
        out.re = (a.re * b.re) - bhVec3_Dot(a.im, b.im);
        return out;
    }

    __forceinline bhQuat bhQuat_Conjugate(bhQuat q)
    {
        bhQuat out = { bhVec3_Negate(q.im), q.re };
        return out;
    }

    __forceinline float bhQuat_Norm(bhQuat q)
    {
        return bhVec3_Length_Sq(q.im) + (q.re * q.re);
    }

    __forceinline bhQuat bhQuat_Inverse(bhQuat q)
    {
        bhQuat conj = bhQuat_Conjugate(q);
        return bhQuat_ScalarMult(conj, 1.f / bhQuat_Norm(q));
    }

    __forceinline bhQuat bhQuat_Rotation(bhVec3 axis, float radians)
    {
        radians /= 2.f;
        bhVec3 na = bhVec3_Normalize(axis);
        bhQuat out = { bhVec3_ScalarMult(na, sinf(radians)), cosf(radians) };
        return out;
    }

    bhMat4 bhQuat_ToMatrix(bhQuat q)
    {
        bhMat4 out;

        float yy2 = (2.f * q.im.y * q.im.y);
        float zz2 = (2.f * q.im.z * q.im.z);
        float xy2 = (2.f * q.im.x * q.im.y);
        float wz2 = (2.f * q.re * q.im.z);
        float xz2 = (2.f * q.im.x * q.im.z);
        float wy2 = (2.f * q.re * q.im.y);

        out.m[0][0] = 1.f - yy2 - zz2;
        out.m[1][0] = xy2 + wz2;
        out.m[2][0] = xz2 - wy2;
        //out.m[3][0] = 0.f; // Translation x

        float xx2 = (2.f * q.im.x * q.im.x);
        float yz2 = (2.f * q.im.y * q.im.z);
        float wx2 = (2.f * q.re * q.im.x);

        out.m[0][1] = xy2 - wz2;
        out.m[1][1] = 1.f - xx2 - zz2;
        out.m[2][1] = yz2 + wx2;
        //out.m[3][1] = 0.f; // Translation y

        out.m[0][2] = xz2 + wy2;
        out.m[1][2] = yz2 - wx2;
        out.m[2][2] = 1.f - xx2 - yy2;
        //out.m[3][2] = 0.f; // Translation z

        out.m[0][3] = 0.f;
        out.m[1][3] = 0.f;
        out.m[2][3] = 0.f;
        out.m[3][3] = 1.f;

        return out;
    }

    __forceinline bhMat4 bhQuat_Rotation_Matrix(bhVec3 axis, float radians)
    {
        bhQuat qRotation = bhQuat_Rotation(axis, radians);
        return bhQuat_ToMatrix(qRotation);
    }

    ////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif //BH_QUAT_H
