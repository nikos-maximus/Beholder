#include <stdio.h>
#include "bhDefines.h"
#include "bhConfig.h"

#define WINDOW_FLAG_FULLSCREEN			BH_BIT(0)
#define WINDOW_FLAG_VSYNC				BH_BIT(1)
#define WINDOW_FLAG_USE_DESKTOP_MODE	BH_BIT(2)

#define RENDER_FLAG_GL_USE_SPIRV		BH_BIT(0)
#define RENDER_FLAG_GEN_MIPMAPS			BH_BIT(0)

static bhConfig g_config;

////////////////////////////////////////////////////////////////////////////////
bhWindowSettings bhWindowSettings_Init()
{
	bhWindowSettings ws = 
	{
		.width = 1280,
		.height = 768,
		.refresh_rate = 60
	};
	ws.flags = WINDOW_FLAG_VSYNC;
	return ws;
}

const bhWindowSettings* bhConfig_GetWindowSettings()
{
	return &(g_config.windowSettings);
}

void bhConfig_SetWindowSettings(const bhWindowSettings* ws)
{
	g_config.windowSettings = *ws;
}

int bhConfig_UseDesktopMode(const bhWindowSettings* ws)
{
	return ws->flags & WINDOW_FLAG_USE_DESKTOP_MODE;
}

int bhConfig_UseFullscreen(const bhWindowSettings* ws)
{
	return ws->flags & WINDOW_FLAG_FULLSCREEN;
}

int bhConfig_UseVSync(const bhWindowSettings* ws)
{
	return ws->flags & WINDOW_FLAG_VSYNC;
}

////////////////////////////////////////////////////////////////////////////////
bhRenderSettings bhRenderSettings_Init()
{
	bhRenderSettings rs =
	{
		.gl.versionMajor = 4,
		.gl.versionMinor = 6,
		.gl.flags = RENDER_FLAG_GL_USE_SPIRV,

		.vk.versionMajor = 1,
		.vk.versionMinor = 1,
		.vk.versionPatch = 0,

		.color_depth = 32,
		.depth_bits = 24,
		.stencil_bits = 8,
		.aa_samples = 0,
		.anisotropy_level = 0,
		.num_swapchain_images = 2,
		.flags = RENDER_FLAG_GEN_MIPMAPS,
	};
	return rs;
}

const bhRenderSettings* bhConfig_GetRenderSettings()
{ 
	return &(g_config.renderSettings);
}

void bhConfig_SetRenderSettings(const bhRenderSettings* rs)
{
	g_config.renderSettings = *rs;
}

int bhConfig_GenMipmaps(const bhRenderSettings* rs)
{
	return rs->flags & RENDER_FLAG_GEN_MIPMAPS;
}

int bhConfig_GL_UseCompiledShaders(const bhRenderSettings* rs)
{
	return rs->flags & RENDER_FLAG_GL_USE_SPIRV;
}

////////////////////////////////////////////////////////////////////////////////
bhAudioSettings bhAudioSettings_Init()
{
	bhAudioSettings as =
	{
		.master_gain = 1.0f
	};
	return as;
}

const bhAudioSettings* bhConfig_GetAudioSettings()
{
	return &(g_config.audioSettings);
}

void bhConfig_SetAudioSettings(const bhAudioSettings* as)
{
	g_config.audioSettings = *as;
}

////////////////////////////////////////////////////////////////////////////////
bhInputSettings bhInputSettings_Init()
{
	bhInputSettings is =
	{
		.mouse_sensitivity = 1.0f
	};
	return is;
}

const bhInputSettings* bhConfig_GetInputSettings()
{
	return &(g_config.inputSettings);
}

void bhConfig_SetInputSettings(const bhInputSettings* is)
{
	g_config.inputSettings = *is;
}

////////////////////////////////////////////////////////////////////////////////
bhSystemSettings bhSystemSettings_Init()
{
	bhSystemSettings ss =
	{
		.memory.max_images = 8,
		.memory.max_meshes = 8,
		.memory.max_materials = 8,
		.memory.max_textures = 8,
		.memory.max_pipelines = 8,

		.threads.num_threads = 4,
		.threads.tasks_per_thread = 64
	};
	return ss;
}

const bhSystemSettings* bhConfig_GetSystemSettings()
{
	return &(g_config.systemSettings);
}

void bhConfig_SetSystemSettings(const bhSystemSettings* ss)
{
	g_config.systemSettings = *ss;
}

////////////////////////////////////////////////////////////////////////////////
bhConfig bhConfig_Init()
{
	bhConfig cfg;
	cfg.windowSettings = bhWindowSettings_Init();
	cfg.renderSettings = bhRenderSettings_Init();
	cfg.audioSettings = bhAudioSettings_Init();
	cfg.inputSettings = bhInputSettings_Init();
	cfg.systemSettings = bhSystemSettings_Init();
	return cfg;
}

void bhConfig_InitGlobal()
{
	g_config = bhConfig_Init();
}

int bhConfig_Load(const char* path)
{
	FILE* file = NULL;
	int result = fopen_s(&file, path, "rb");
	if (result == 0)
	{
		fread(&g_config, sizeof(bhConfig), 1, file);
		fclose(file);
	}
	return result;
}

int bhConfig_Save(const char* path)
{
	FILE* file = NULL;
	int result = fopen_s(&file, path, "wb");
	if (result == 0)
	{
		fwrite(&g_config, sizeof(bhConfig), 1, file);
		fclose(file);
	}
	return result;
}
