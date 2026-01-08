#ifndef BH_VEC3_HPP
#define BH_VEC3_HPP

#ifdef BH_USE_GLM

#include <glm/vec3.hpp>
using bhVec3f = glm::vec3;

#else

#include "bhTypes.hpp"

struct bhVec3f
{
	union
	{
		struct
		{
			bhFloat_t x, y, z;
		};
		struct
		{
			bhFloat_t r, g, b;
		};
		bhFloat_t _values[3];
	};

	bhVec3f()
	{
		_values[0] = _values[1] = _values[2] = bhFloat_t(0);
	}

	bhVec3f(bhFloat_t x, bhFloat_t y, bhFloat_t z)
	{
		_values[0] = x;
		_values[1] = y;
		_values[2] = z;
	}

	bhVec3f operator+(const bhVec3f& v) const
	{
		return bhVec3f(x * v.x, y * v.y, z * v.z);
	}

	bhVec3f operator*(bhFloat_t t) const
	{
		return bhVec3f(x * t, y * t, z * t);
	}
};

#endif //BH_USE_GLM

#endif //BH_VEC3_HPP
