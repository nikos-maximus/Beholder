#include "GL/bhGLTexture.hpp"

void bhGLTexture::GenerateMipmaps()
{
  glBindTexture(target, id);
  glGenerateMipmap(target);
  glBindTexture(target, GL_ZERO);
}
