#ifndef BH_GL_CORE_HPP
#define BH_GL_CORE_HPP

#include <vector>
#include <string>
#include <GL/gl3w.h>
#include "bhTypes.hpp"

////////////////////////////////////////////////////////////////////////////////
struct bhTextureGL
{
    bhTextureGL() = default;

    bhTextureGL(GLuint _id, GLenum _target)
        : id(_id)
        , target(_target)
    {}

    GLuint id { GL_ZERO };
    GLenum target { GL_NONE };
};

////////////////////////////////////////////////////////////////////////////////
struct bhBufferGL
{
    GLuint buffer{ GL_ZERO };
    GLsizeiptr size{ 0 };
};

////////////////////////////////////////////////////////////////////////////////
struct GLFormats
{
    GLenum fmt{ GL_NONE };
    GLint internalFmt{ GL_NONE };
};

////////////////////////////////////////////////////////////////////////////////
struct bhFramebufferGL
{
    bhSize2Di size;
    std::vector<GLuint> colorAttachments_v;
    GLuint depthStencilAttachment{ GL_ZERO };
    GLuint framebuffer{ GL_ZERO };
};

////////////////////////////////////////////////////////////////////////////////
GLenum bhGL_GetShaderType(bhShaderType type);
bhShaderType bhGL_CreateShaderFromFile(const char* fileName, GLuint& newShader);
GLuint bhGL_CreateProgramFromShaders(const std::vector<std::string>* shaderNames_v);

////////////////////////////////////////////////////////////////////////////////
#ifdef BH_API_DEBUG

GLboolean bhGL_InitDebugCallback();
void bhGL_DestroyDebugCallback();

#endif //BH_API_DEBUG

#endif //BH_GL_CORE_HPP
