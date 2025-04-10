#include "bhMathUtil.hpp"

int bhMath_GetPrevPowerOf2(unsigned int val)
{
  if (bhMath_IsPowerOf2(val))
  {
    return val;
  }
  unsigned int pow = 0;
  unsigned int p = 0;
  while (val > p)
  {
    p = (1 << pow);
    ++pow;
  }
  return 1 << pow;
}

int bhMath_GetNextPowerOf2(unsigned int val)
{
  if (bhMath_IsPowerOf2(val))
  {
    return val;
  }
  return bhMath_GetPrevPowerOf2(val) << 1;
}

int bhMath_GetPowerOf2(unsigned int val)
{
  if (!bhMath_IsPowerOf2(val))
  {
    return -1;
  }
  unsigned int pow = 0;
  while (val > 0)
  {
    val = (val >> 1);
    ++pow;
  }
  return pow;
}
