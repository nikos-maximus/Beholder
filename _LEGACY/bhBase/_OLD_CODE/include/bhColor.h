#pragma once
#include <inttypes.h>

////////////////////////////////////////////////////////////////////////////////
#define COLOR_UB_MIN 0
#define COLOR_UB_MAX UINT8_MAX

#define COLOR_F_MIN 0.f
#define COLOR_F_MAX 1.f

template<typename Component_t> struct bhColor4
{
	bhColor4()
		: r(Component_t(0)), g(Component_t(0)), b(Component_t(0)), a(Component_t(0))
	{}

	bhColor4(Component_t _r, Component_t _g, Component_t _b, Component_t _a)
		: r(_r), g(_g), b(_b), a(_a)
	{}

	Component_t r, g, b, a;
};

////////////////////////////////////////////////////////////////////////////////
typedef bhColor4<uint8_t> bhColor4ub;

////////////////////////////////////////////////////////////////////////////////
typedef bhColor4<float> bhColor4f;
