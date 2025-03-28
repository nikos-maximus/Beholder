#ifndef BH_CONFIG_H
#define BH_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
typedef struct bhWindowSettings
{
	int width, height;
	int refresh_rate;
	int display_index;
	int flags;
}
bhWindowSettings;

bhWindowSettings bhWindowSettings_Init();
const bhWindowSettings* bhConfig_GetWindowSettings();
void bhConfig_SetWindowSettings(const bhWindowSettings* ws);
int bhConfig_UseDesktopMode(const bhWindowSettings* ws);
int bhConfig_UseFullscreen(const bhWindowSettings* ws);
int bhConfig_UseVSync(const bhWindowSettings* ws);

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int versionMajor;
	int versionMinor;
	int flags;
}
bhGLSettings;

typedef struct
{
	int versionMajor;
	int versionMinor;
	int versionPatch;
}
bhVKSettings;

typedef struct
{
	bhGLSettings gl;
	bhVKSettings vk;
	
	int color_depth; // Accept only 32-bit color configurations
	int depth_bits;
	int stencil_bits;
	int aa_samples;
	int anisotropy_level; // Anisotropy value = (1 << anisotropy_level) > 1 ? (1 << anisotropy_level) : 0, as a value of 1 is meaningless
	int num_swapchain_images;
	int flags;
}
bhRenderSettings;

bhRenderSettings bhRenderSettings_Init();
const bhRenderSettings* bhConfig_GetRenderSettings();
void bhConfig_SetRenderSettings(const bhRenderSettings* rs);
int bhConfig_GenMipmaps(const bhRenderSettings* rs);
int bhConfig_GL_UseCompiledShaders(const bhRenderSettings* rs);

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float master_gain;
}
bhAudioSettings;

bhAudioSettings bhAudioSettings_Init();
const bhAudioSettings* bhConfig_GetAudioSettings();
void bhConfig_SetAudioSettings(const bhAudioSettings* as);

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float mouse_sensitivity;
}
bhInputSettings;

bhInputSettings bhInputSettings_Init();
const bhInputSettings* bhConfig_GetInputSettings();
void bhConfig_SetInputSettings(const bhInputSettings* is);

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int max_images;
	int max_meshes;
	int max_materials;
	int max_textures;
	int max_pipelines;
}
bhMemorySettings;

typedef struct
{
	int num_threads;
	int tasks_per_thread;
}
bhThreadsSettings;

typedef struct
{
	bhMemorySettings memory;
	bhThreadsSettings threads;
}
bhSystemSettings;

bhSystemSettings bhSystemSettings_Init();
const bhSystemSettings* bhConfig_GetSystemSettings();
void bhConfig_SetSystemSettings(const bhSystemSettings* ss);

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	bhWindowSettings windowSettings;
	bhRenderSettings renderSettings;
	bhAudioSettings audioSettings;
	bhInputSettings inputSettings;
	bhSystemSettings systemSettings;
}
bhConfig;

//bhConfig bhConfig_Init();

void bhConfig_InitGlobal();
int bhConfig_Load(const char* path);
int bhConfig_Save(const char* path);

#ifdef __cplusplus
}
#endif //extern "C"

#endif //BH_CONFIG_H
