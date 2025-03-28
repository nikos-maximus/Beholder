#ifndef BH_GL_TEXTURE_HPP
#define BH_GL_TEXTURE_HPP

#include "GL/bhGLIncludes.hpp"
#include "texture/bhTexture.hpp"

class bhGLTexture : public bhTexture
{
public:
  //bool Load(const char* fileName);
  //const bhImage* GetImage() const { return image.get(); }
  //void SetTexture(const DeviceTexture& _dt) { texture = _dt; }
  //const DeviceTexture& GetTexture() const { return texture; }
  //DeviceTexture& GetTexture() { return texture; }

  __forceinline bool IsValid() override final
  {
    return ((glIsTexture(id) == GL_TRUE) && (target != GL_NONE));
  }

  void GenerateMipmaps();

protected:
private:
  GLuint id{ GL_ZERO };
  GLenum target{ GL_NONE };
};

#endif //BH_GL_TEXTURE_HPP
