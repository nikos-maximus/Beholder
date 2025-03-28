#pragma once
#define ENABLE_XML_IMPORT 0

#include "bhTypes.h"
#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <assimp/color4.h>

#if ENABLE_XML_IMPORT
#include "tinyxml2/tinyxml2.h"
#endif

template<typename T> inline glm::tvec2<T> FromAiVector2(aiVector2t<T> const& v)
{
	return glm::tvec2<T>(v.x, v.y);
}

template<typename T> inline glm::tvec3<T> FromAiVector3(aiVector3t<T> const& v)
{
	return glm::tvec3<T>(v.x, v.y, v.z);
}

inline glm::vec3 FromAiColor3(aiColor3D const& c)
{
	return glm::vec3(c.r, c.g, c.b);
}

#if ENABLE_XML_IMPORT

void ReadVector(glm::vec2* v, tinyxml2::XMLElement const* el)
{
	v->x = el->FloatAttribute("x", 0.0f);
	v->y = el->FloatAttribute("y", 0.0f);
}

void ReadVector(glm::vec3* v, tinyxml2::XMLElement const* el)
{
	v->x = el->FloatAttribute("x", 0.0f);
	v->y = el->FloatAttribute("y", 0.0f);
	v->z = el->FloatAttribute("z", 0.0f);
}

void ReadVector(glm::vec4* v, tinyxml2::XMLElement const* el)
{
	v->x = el->FloatAttribute("x", 0.0f);
	v->y = el->FloatAttribute("y", 0.0f);
	v->z = el->FloatAttribute("z", 0.0f);
	v->w = el->FloatAttribute("w", 1.0f);
}

void ReadColor(glm::vec3* color, tinyxml2::XMLElement const* el)
{
	color->r = el->FloatAttribute("r", 1.0f);
	color->g = el->FloatAttribute("g", 0.0f);
	color->b = el->FloatAttribute("b", 1.0f);
}

void ReadColor(glm::vec4* color, tinyxml2::XMLElement const* el)
{
	color->r = el->FloatAttribute("r", 1.0f);
	color->g = el->FloatAttribute("g", 0.0f);
	color->b = el->FloatAttribute("b", 1.0f);
	color->a = el->FloatAttribute("a", 1.0f);
}

#endif

#if 0

inline bhVec3 bhVector3Type(aiVector3D const& v)
{
	return bhVec3(v.x, v.y, v.z);
}

inline bhColor bhColorType(aiColor3D const& c)
{
	return bhColor(c.r, c.g, c.b);
}

#endif
