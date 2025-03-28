#ifndef BH_CONFIG_H
#define BH_CONFIG_H

#include <SDL3/SDL_properties.h>

struct bhConfig
{
	////////////////////////////////////////
	struct WindowSettings
	{
		WindowSettings()
			: w(1280), h(768), refresh_rate(6), display_index(SDL_MAX_UINT32)
			, use_vsync(false), use_desktop_mode(false)
		{}

		Uint32 display_index{ SDL_MAX_UINT32 };
		Uint16 w, h, refresh_rate;
		bool fullscreen : 1;
		bool use_vsync : 1;
		bool use_desktop_mode : 1;
	};

	////////////////////////////////////////
	struct RenderSettings
	{
		struct VKSettings
		{
			VKSettings()
				:versionMajor(1), versionMinor(1), versionPatch(0)
			{}

			Uint8 versionMajor, versionMinor, versionPatch;
		};

		RenderSettings()
			: color_depth(32), depth_bits(24), stencil_bits(8)
			, aa_samples(0), anisotropy_level(0), num_swapchain_images(2)
			,gen_mipmaps(true)
		{}

		VKSettings vk;

		Uint8 color_depth; // Accept only 32-bit color configurations
		Uint8 depth_bits;
		Uint8 stencil_bits;
		Uint8 aa_samples;
		Uint8 anisotropy_level; // Anisotropy value = (1 << anisotropy_level) > 1 ? (1 << anisotropy_level) : 0, as a value of 1 is meaningless
		Uint8 num_swapchain_images;
		bool gen_mipmaps : 1;
	};

	////////////////////////////////////////
	struct AudioSettings
	{
		AudioSettings()
			: master_gain(1.0f)
		{}

		float master_gain;
	};

	////////////////////////////////////////
	struct InputSettings
	{
		InputSettings()
			: mouse_sensitivity(1.0f)
		{}

		float mouse_sensitivity;
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
