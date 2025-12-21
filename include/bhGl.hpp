#ifndef BH_GL_HPP
#define BH_GL_HPP
#include <GL/gl3w.h>

namespace bhGl
{
	bool Init();
	void BeginFrame();
	void EndFrame();
	bool InitImGui(SDL_Window* window, void* glContext);
	void DestroyImGui();
	void ShowImGui(bool show);
}

#endif //BH_GL_HPP
