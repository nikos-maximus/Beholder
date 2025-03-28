#ifndef BH_MAT4_HPP
#define BH_MAT4_HPP

#include "bhVec4.hpp"

struct bhFloatMat4
{
	union
	{
		bhFloat4 _cols[4];
		bhFloat_t _valuesL[16];
		bhFloat_t _valuesG[4][4];
	};

	bhFloatMat4()
	{
		for (size_t c = 0; c < 4; ++c)
		{
			for (size_t r = 0; r < 4; ++r)
			{
				_valuesG[c][r] = (r == c) ? bhFloat_t(1) : bhFloat_t(0);
			}
		}
	}
};

#endif //BH_MAT4_HPP
