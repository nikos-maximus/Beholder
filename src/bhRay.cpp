#include "bhRay.hpp"

bhFloat3 bhRay::At(bhFloat_t t) const
{
  return orig + dir * t;
}
