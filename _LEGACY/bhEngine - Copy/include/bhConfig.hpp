#ifndef BH_CONFIG_H
#define BH_CONFIG_H

#include "SDL3/SDL_stdinc.h"
#include "bhTypes.hpp"

#define WINDOW_FLAG_FULLSCREEN BH_BIT(0)
#define WINDOW_FLAG_VSYNC BH_BIT(1)
#define WINDOW_FLAG_USE_DESKTOP_MODE BH_BIT(2)

#define RENDER_FLAG_GL_USE_SPIRV BH_BIT(0)
#define RENDER_FLAG_GEN_MIPMAPS BH_BIT(0)

struct bhConfig
{
	////////////////////////////////////////
	struct WindowSettings
	{
		int w{ 1280 }, h{ 768 };
		int refresh_rate{ 60 };
		Uint32 display_index{ SDL_MAX_UINT32 };

		inline int UseDesktopMode() const { return flags & WINDOW_FLAG_USE_DESKTOP_MODE; }
		inline int UseFullscreen() const { return flags & WINDOW_FLAG_FULLSCREEN; }
		inline int UseVSync() const { return flags & WINDOW_FLAG_VSYNC; }

	private:
		int flags{ WINDOW_FLAG_VSYNC };
	};

	////////////////////////////////////////
	struct RenderSettings
	{
		struct GLSettings
		{
			int versionMajor{ 4 };
			int versionMinor{ 6 };

			int UseCompiledShaders() const { return flags & RENDER_FLAG_GL_USE_SPIRV; }

		private:
			int flags{ RENDER_FLAG_GL_USE_SPIRV };
		};

		struct VKSettings
		{
			int versionMajor{ 1 };
			int versionMinor{ 1 };
			int versionPatch{ 0 };
		};

		GLSettings gl;
		VKSettings vk;

		bhGpuApi api{ bhGpuApi::BH_VULKAN };
		int color_depth{ 32 }; // Accept only 32-bit color configurations
		int depth_bits{ 24 };
		int stencil_bits{ 8 };
		int aa_samples{ 0 };
		int anisotropy_level{ 0 }; // Anisotropy value = (1 << anisotropy_level) > 1 ? (1 << anisotropy_level) : 0, as a value of 1 is meaningless
		int num_swapchain_images{ 2 };

		int GenMipmaps() const { return flags & RENDER_FLAG_GEN_MIPMAPS; }

	private:
		int flags{ RENDER_FLAG_GEN_MIPMAPS };
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
	struct SystemSettings
	{
		struct MemorySettings
		{
			int max_images{ 8 };
			int max_meshes{ 8 };
			int max_materials{ 8 };
			int max_textures{ 8 };
			int max_pipelines{ 8 };
		};

		struct ThreadsSettings
		{
			int num_threads{ 4 };
			int tasks_per_thread{ 64 };
		};

		MemorySettings memory;
		ThreadsSettings threads;
	};

	WindowSettings windowSt;
	RenderSettings renderSt;
	AudioSettings audioSt;
	InputSettings inputSt;
	SystemSettings systemSt;

	static int Load(const char* path, bhConfig& cfg);
	static int Save(const char* path, const bhConfig& cfg);
};

#endif //BH_CONFIG_H
