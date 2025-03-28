#ifndef BH_RAY_HPP
#define BH_RAY_HPP

#include "bhVec3.hpp"

class bhRay
{
public:
  bhFloat3 At(bhFloat_t t) const;

protected:
private:
  bhFloat3 orig;
  bhFloat3 dir;
};

#endif //BH_RAY_HPP