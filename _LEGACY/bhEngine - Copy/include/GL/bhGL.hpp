#ifndef BH_GL_HPP
#define BH_GL_HPP

#include "GL/bhGLIncludes.hpp"
#include "bhTypes.hpp"

namespace bhGL
{
  struct Buffer
  {
    GLuint buffer{ GL_ZERO };
    GLsizeiptr size{ 0 };
  };

  bool CopyDataToBuffer(const Buffer* buffer, GLintptr offset, GLsizei reqSize, const void* data);
}

#endif //BH_GL_HPP
