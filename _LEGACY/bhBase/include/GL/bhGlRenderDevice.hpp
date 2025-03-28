#ifndef BH_GL_RENDER_DEVICE_HPP
#define BH_GL_RENDER_DEVICE_HPP

#include "GL/bhGl.hpp"

struct SDL_Window;

namespace bhGl
{
	class RenderDevice
	{
	public:
		RenderDevice(SDL_Window* _wnd);
		GLboolean Init();
		void InitSDL_GL();
		void Destroy();
		void BeginFrame();
		void EndFrame();

	protected:
	private:
		SDL_Window* wnd{ nullptr };
		void* sdlGLContext{ nullptr };
	};
}

#endif //BH_GL_RENDER_DEVICE_HPP
