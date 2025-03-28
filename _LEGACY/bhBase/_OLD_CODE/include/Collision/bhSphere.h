#ifndef BH_SPHERE_H
#define BH_SPHERE_H

#include "Math/bhVector.h"

template<typename Scalar_t>
struct bhSphere
{
	bhVec3<Scalar_t> center;
	Scalar_t radius = Scalar_t(0);

	bhSphere(Scalar_t _radius)
		: radius(_radius)
	{}

	bhSphere(bhVec3<Scalar_t> const& _center, Scalar_t _radius)
		: center(_center)
		, radius(_radius)
	{}

	bool Intersects(bhSphere const& s)
	{
		return DistanceSquared(center, s.center) < (radius + s.radius) * (radius + s.radius);
	}
};

#endif //BH_SPHERE_H
