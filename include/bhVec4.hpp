#ifndef BH_SCALAR4_HPP
#define BH_SCALAR4_HPP

#include "bhTypes.hpp"

struct bhFloat4
{
	union
	{
		struct
		{
			bhFloat_t x, y, z, w;
		};
		struct
		{
			bhFloat_t r, g, b, a;
		};
		bhFloat_t _values[4] = {};
	};

	bhFloat4()
	{
		_values[0] = _values[1] = _values[2] = bhFloat_t(0);
		_values[3] = bhFloat_t(1);
	}
	
	bhFloat4(bhFloat_t x, bhFloat_t y, bhFloat_t z)
	{
		_values[0] = x;
		_values[1] = y;
		_values[2] = z;
		_values[3] = bhFloat_t(1);
	}
	
	bhFloat4(bhFloat_t x, bhFloat_t y, bhFloat_t z, bhFloat_t w)
	{
		_values[0] = x;
		_values[1] = y;
		_values[2] = z;
		_values[3] = w;
	}
};

#endif //BH_SCALAR4_HPP
