#ifndef BH_SCALAR3_HPP
#define BH_SCALAR3_HPP

#include "bhTypes.hpp"

struct bhFloat3
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

	bhFloat3()
	{
		_values[0] = _values[1] = _values[2] = bhFloat_t(0);
	}

	bhFloat3(bhFloat_t x, bhFloat_t y, bhFloat_t z)
	{
		_values[0] = x;
		_values[1] = y;
		_values[2] = z;
	}

	bhFloat3 operator+(const bhFloat3& v) const
	{
		return bhFloat3(x * v.x, y * v.y, z * v.z);
	}

	bhFloat3 operator*(bhFloat_t t) const
	{
		return bhFloat3(x * t, y * t, z * t);
	}
};

#endif //BH_SCALAR3_HPP
