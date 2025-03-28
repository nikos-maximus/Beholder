#ifndef BH_SCALAR2_HPP
#define BH_SCALAR2_HPP

#include "bhTypes.hpp"

struct bhFloat2
{
	union
	{
		struct
		{
			bhFloat_t x, y;
		};
		struct
		{
			bhFloat_t r, g;
		};
		bhFloat_t _values[2];
	};

	bhFloat2()
	{
		_values[0] = _values[1] = bhFloat_t(0);
	}

	bhFloat2(bhFloat_t x, bhFloat_t y)
	{
		_values[0] = x;
		_values[1] = y;
	}
};

#endif //BH_SCALAR2_HPP
