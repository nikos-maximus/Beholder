#ifndef BH_MATH_HPP
#define BH_MATH_HPP

#define _USE_MATH_DEFINES
#include <math.h>

namespace bhMath
{
  template<typename Scalar_t>
  inline Scalar_t Min(Scalar_t a, Scalar_t b)
  {
    if (a < b)
      return a;
    return b;
  }

  template<typename Scalar_t>
  inline Scalar_t Max(Scalar_t a, Scalar_t b)
  {
    if (a > b)
      return a;
    return b;
  }

  __forceinline float Deg2Rad(float degrees)
  {
    static float mod = (float)M_PI / 180.0f;
    return degrees * mod;
  }

  __forceinline float Rad2Deg(float radians)
  {
    static float mod = 180.0f / (float)M_PI;
    return radians * mod;
  }

  __forceinline int IsPowerOf2(int val)
  {
    return ((val & (val - 1)) == 0) ? 1 : 0;
  }

  __forceinline int Abs(int val)
  {
    return val < 0 ? -val : val;
  }

  __forceinline int Roundf(float v)
  {
    return (v >= 0.0f) ? (int)(v + 0.5f) : (int)(v - 0.5f);
  }

  __forceinline int Floorf(float v)
  {
    return (int)v;
  }

  // TODO:
  //__forceinline int Ceilf(float v)
  //{
  //    return Floorf(v) + 1;
  //}

  int GetPrevPowerOf2(unsigned int val);
  int GetNextPowerOf2(unsigned int val);
  int GetPowerOf2(unsigned int val);
}

#endif //BH_MATH_HPP
