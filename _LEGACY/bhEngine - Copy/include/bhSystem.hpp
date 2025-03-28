#ifndef BH_SYSTEM_HPP
#define BH_SYSTEM_HPP

//#ifdef BH_GPU_API_VULKAN
//	#define BH_APP_NAME "bhEngine (Vulkan)"
//#elif BH_GPU_API_OPENGL
//	#define BH_APP_NAME "bhEngine (OpenGL)"
//#endif

#define BH_APP_NAME "bhEngine"

#include "bhGPUContext.hpp"

class bhMeshCache;
struct SDL_Window;
struct bhConfig;

namespace bhSystem
{
	SDL_Window* MainWindow();
	bhGPUContext* GPUContext();
	bhMeshCache* MeshCache();

	const bhConfig* Config();
	bool Init(char* argv[]);
	void Destroy();
	const char* EngineName();
	uint32_t EngineVersion();
	inline const char* ApplicationName() { return BH_APP_NAME; }
	uint32_t ApplicationVersion();
	float WindowASpect();
	bool CreateMainWindow();
};

#endif //BH_SYSTEM_HPP
