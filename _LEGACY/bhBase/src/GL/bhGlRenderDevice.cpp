#include "GL/bhGlRenderDevice.hpp"
#include <SDL_video.h>
#include "bhConfig.h"

namespace bhGl
{
	RenderDevice::RenderDevice(SDL_Window* _wnd)
		: wnd(_wnd)
	{}

	GLboolean RenderDevice::Init()
	{
		sdlGLContext = SDL_GL_CreateContext(wnd);

		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClearDepth(1.0);

		//glFrontFace(GL_CCW); // Default
		glCullFace(GL_BACK);
		//glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_LINE);

		return GL_TRUE;
	}

	void RenderDevice::InitSDL_GL()
	{
		const bhRenderSettings* rs = bhConfig_GetRenderSettings();
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, rs->gl.versionMajor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, rs->gl.versionMinor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	#ifdef _DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	#endif
	}

	void RenderDevice::Destroy()
	{
		SDL_GL_DeleteContext(sdlGLContext);
	}

	void RenderDevice::BeginFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderDevice::EndFrame()
	{
		glFlush();
		//glFinish(); // For benchmarking?
		SDL_GL_SwapWindow(wnd);
	}
}
