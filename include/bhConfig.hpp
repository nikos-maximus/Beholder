#ifndef BH_CONFIG_H
#define BH_CONFIG_H

#include <SDL3/SDL_properties.h>

struct bhConfig
{
	////////////////////////////////////////
	struct WindowSettings
	{
		Uint32 display_index{ SDL_MAX_UINT32 };
		Uint16 w{ 1280 }, h{ 768 }, refresh_rate{ 60 };
		bool fullscreen{ false };
		bool use_vsync{ true };
		bool use_desktop_mode{ true };
	};

	////////////////////////////////////////
	struct RenderSettings
	{
		Uint8 color_depth{ 32 }; // Accept only 32-bit color configurations
		Uint8 depth_bits{ 24 };
		Uint8 stencil_bits{ 8 };
		Uint8 aa_samples{ 0 };
		Uint8 anisotropy_level{ 0 }; // Anisotropy value = (1 << anisotropy_level) > 1 ? (1 << anisotropy_level) : 0, as a value of 1 is meaningless
		bool gen_mipmaps{ true };
	};

	////////////////////////////////////////
	struct AudioSettings
	{
		float master_gain{ 1.0f };
	};

	////////////////////////////////////////
	struct InputSettings
	{
		float mouse_sensitivity{ 1.0f };
	};

	////////////////////////////////////////
	WindowSettings window;
	RenderSettings render;
	AudioSettings audio;
	InputSettings input;

	static int Load(const char* path, bhConfig& cfg);
	static int Save(const char* path, const bhConfig& cfg);
	static SDL_PropertiesID CreateProperties(const bhConfig& config);
	static void DestroyProperties(SDL_PropertiesID props);
};

#endif //BH_CONFIG_H
