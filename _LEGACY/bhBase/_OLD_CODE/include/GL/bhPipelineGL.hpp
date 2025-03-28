#ifndef BH_WORLD_PIPELINE_GL_HPP
#define BH_WORLD_PIPELINE_GL_HPP

#include "GL/bhStructsGL.hpp"

////////////////////////////////////////////////////////////////////////////////
struct bhTextureInputGL
{
    bhTextureGL texture;
    uint32_t slot { UINT_MAX };
};

struct bhMaterialGL_World
{
    bhTextureInputGL textureInputs_v[2];
};

////////////////////////////////////////////////////////////////////////////////
struct bhPipelineGL_World
{
    GLuint programID{ 0 };

    bhBufferGL vertexBuffer;
    GLintptr vertexBufferWriteOffset{ 0 };
    GLint baseVertex{ 0 };
    bhBufferGL indexBuffer;
    GLintptr indexBufferWriteOffset{ 0 };
    GLint baseIndex{ 0 };
    GLuint viewDataBuffer { 0 };
    GLuint modelBuffer { 0 };
    GLuint VAO{ 0 };
};

#endif //BH_WORLD_PIPELINE_GL_HPP
