#ifndef BH_GL_HPP
#define BH_GL_HPP

#include <GL/gl3w.h>

namespace bhGl
{
	struct Texture
	{
		GLuint id{ GL_ZERO };
		GLenum target{ GL_NONE };
	};
}

#endif //BH_GL_HPP
