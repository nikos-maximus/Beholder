#ifndef BH_GRAPHICS_COMMON_HPP
#define BH_GRAPHICS_COMMON_HPP

#include <string>

////////////////////////////////////////////////////////////////////////////////
enum bhShaderBindings
{
	SHADER_BINDING_VIEW_PROJECTION,
	SHADER_BINDING_MODEL_TRANSFORM,

	//SHADER_BINDING_DIFFUSE_MAP,
	//SHADER_BINDING_SPECULAR_MAP,
	//SHADER_BINDING_NORMAL_MAP,

	//SHADER_BINDING_LIGHT_POSITION,
	//SHADER_BINDING_LIGHT_DIRECTION,
	//SHADER_BINDING_LIGHT_AMBIENT,
	//SHADER_BINDING_LIGHT_DIFFUSE,
	//SHADER_BINDING_LIGHT_SPECULAR,
	//SHADER_BINDING_LIGHT_RADIUS,
	//SHADER_BINDING_LIGHT_SPOT_INNER_CONE,
	//SHADER_BINDING_LIGHT_SPOT_OUTER_CONE,
	//SHADER_BINDING_LIGHT_ATTENUATION,

	NUM_SHADER_BINDINGS
};

////////////////////////////////////////////////////////////////////////////////
enum bhSamplerSlots
{
	SAMPLER_SLOT_ALBEDO,
	SAMPLER_SLOT_NORMAL_SPECULAR,

	NUM_SAMPLER_SLOTS
};

////////////////////////////////////////////////////////////////////////////////
struct bhWorldMaterialCreateInfo
{
	std::string texture_Albedo;
	std::string texture_NormalSpecular;
};

#endif //BH_GRAPHICS_COMMON_HPP
