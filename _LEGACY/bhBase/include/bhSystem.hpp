#ifndef BH_SYSTEM_HPP
#define BH_SYSTEM_HPP

#include <memory>
#include <SDL.h>

#ifdef BH_GPU_API_VULKAN
	#include "VK/bhVkInstance.hpp"
	#include "VK/bhVkRenderDevice.hpp"
	#define BH_SDL_API_FLAG SDL_WINDOW_VULKAN
	#define BH_APP_NAME "bhEngine (Vulkan)"
	typedef bhVk::RenderDevice bhRenderDevice;
#elif BH_GPU_API_OPENGL
	#include "GL/bhGlRenderDevice.hpp"
	#define BH_SDL_API_FLAG SDL_WINDOW_OPENGL
	#define BH_APP_NAME "bhEngine (OpenGL)"
	typedef bhGl::RenderDevice bhRenderDevice;
#endif

class bhSystem
{
public:
	static bhSystem& Get();
	SDL_Window* MainWindow();
	bhRenderDevice* RenderDevice();
	bool Init(char* argv[]);
	void Destroy();
	const char* EngineName() const;
	uint32_t EngineVersion() const;
	const char* ApplicationName() const { return BH_APP_NAME; }
	uint32_t ApplicationVersion() const;
	float WindowASpect() const;

protected:
private:
	bool CreateMainWindow();

	SDL_Window* mainWnd{ nullptr };

#ifdef BH_GPU_API_VULKAN
	bhVk::Instance vkInst;
#endif

	std::unique_ptr<bhRenderDevice> renderDevice;
};

#endif //BH_SYSTEM_HPP
