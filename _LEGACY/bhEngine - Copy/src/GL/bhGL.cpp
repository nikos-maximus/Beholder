#include <assert.h>
#include "gl/bhGL.hpp"
#include "bhLog.h"
//#include "Texture/bhImage.hpp"

namespace bhGL
{
  bool CopyDataToBuffer(const Buffer* buffer, GLintptr offset, GLsizei reqSize, const void* data)
  {
    if (offset + reqSize > buffer->size)
    {
      bhLog_Message(LP_CRITICAL, "CopyDataToBuffer : Exceeding buffer size!");
      return false;
    }
    glNamedBufferSubData(buffer->buffer, offset, reqSize, data);
    return true;
  }

  GLenum GetShaderStage(bhShaderStage stage)
  {
    GLenum GL_ShaderTypes[NUM_SHADER_STAGES] =
    {
        GL_VERTEX_SHADER,
        GL_TESS_CONTROL_SHADER,
        GL_TESS_EVALUATION_SHADER,
        GL_GEOMETRY_SHADER,
        GL_FRAGMENT_SHADER,
        GL_COMPUTE_SHADER
    };
    return GL_ShaderTypes[(int)stage];
  }
}
