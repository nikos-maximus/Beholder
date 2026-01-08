#ifndef BH_VEC2_HPP
#define BH_VEC2_HPP

#ifdef BH_USE_GLM

#include <glm/vec2.hpp>
using bhVec2f = glm::vec2;

#else

#include "bhTypes.hpp"

struct bhVec2f
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

	bhVec2f()
	{
		_values[0] = _values[1] = bhFloat_t(0);
	}

	bhVec2f(bhFloat_t x, bhFloat_t y)
	{
		_values[0] = x;
		_values[1] = y;
	}
};

#endif //BH_USE_GLM

#endif //BH_VEC2_HPP
