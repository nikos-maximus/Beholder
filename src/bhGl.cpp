#include "bhGl.hpp"

namespace bhGl
{
	bool Init()
	{
		if (gl3wInit()) // 0 on success
		{
			return false;
		}
		if (!gl3wIsSupported(4, 6)) // 1 on success
		{
			return false;
		}

		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClearDepth(1.0);
		glClearStencil(0);

		return true;
	}

	void BeginFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void EndFrame()
	{
		glFlush();
	}
}
