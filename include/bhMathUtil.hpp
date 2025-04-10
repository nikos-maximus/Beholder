#ifndef BH_MATH_HPP
#define BH_MATH_HPP

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

__forceinline float bhMath_Deg2Rad(float degrees)
{
  static float mod = (float)M_PI / 180.0f;
  return degrees * mod;
}

__forceinline float bhMath_Rad2Deg(float radians)
{
  static float mod = 180.0f / (float)M_PI;
  return radians * mod;
}

__forceinline int bhMath_IsPowerOf2(int val)
{
  return ((val & (val - 1)) == 0) ? 1 : 0;
}

__forceinline int bhMath_Abs(int val)
{
  return val < 0 ? -val : val;
}

__forceinline int bhMath_Roundf(float v)
{
  return (v >= 0.0f) ? (int)(v + 0.5f) : (int)(v - 0.5f);
}

__forceinline int bhMath_Floorf(float v)
{
  return (int)v;
}

// TODO:
//__forceinline int bhMath_Ceilf(float v)
//{
//    return bhMath_Floorf(v) + 1;
//}

int bhMath_GetPrevPowerOf2(unsigned int val);
int bhMath_GetNextPowerOf2(unsigned int val);
int bhMath_GetPowerOf2(unsigned int val);

#endif //BH_MATH_HPP
