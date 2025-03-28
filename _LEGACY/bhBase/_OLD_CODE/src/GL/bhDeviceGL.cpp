#include "GL/bhDeviceGL.hpp"
#include "bhConfig.h"
#include "bhImage.h"
#include "bhEnv.h"

#ifdef BH_USE_IMGUI
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"
#endif //BH_USE_IMGUI

////////////////////////////////////////////////////////////////////////////////
bool bhDeviceGL::Init()
{
    bhImage_Init();

    bhEnv_SetApplicationName("Beholder (OpenGL)");
    if (bhCreateMainWindow() == 0)
    {
        GLContext = SDL_GL_CreateContext(bhGetMainWindow());
        assert(GLContext != NULL);
        SDL_GL_MakeCurrent(bhGetMainWindow(), GLContext);

        if (gl3wInit())
        {
            bhLog_Message(LOG_TYPE_CRITICAL, "Could not initialize gl3w");
            return false;
        }

        bhLog_Message(LOG_TYPE_INFO, "OpenGL runtime [Driver] %s [glsl] %s [Device] %s", 
            glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_RENDERER));

    #ifdef BH_API_DEBUG
        GLboolean cbResult = bhGL_InitDebugCallback();
        assert(cbResult == GL_TRUE);
    #endif //BH_API_DEBUG

        glClearColor(1.f, 0.f, 1.f, 1.f);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
        glLineWidth(1.f);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        return true;
    }
    return false;
}

void bhDeviceGL::Destroy()
{
    textureManager.DestroyAll(DestroyTexture);

#ifdef BH_API_DEBUG
    bhGL_DestroyDebugCallback();
#endif //BH_API_DEBUG

    if (GLContext)
    {
        SDL_GL_DeleteContext(GLContext);
    }
    bhDestroyMainWindow();
}

void bhDeviceGL::BeginFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bhDeviceGL::EndFrame()
{
#if BH_RENDERING_PROFILING
    glFinish();
#else
    glFlush();
#endif

    SDL_GL_SwapWindow(bhGetMainWindow());
    IncFrameCount();
}

float bhDeviceGL::GetViewportAspect()
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    return float(vp[2]) / float(vp[3]);
}

GLenum bhDeviceGL::MapImageTypeToOpenGL(int dim)
{
    switch (dim)
    {
        case 1:
        {
            return GL_TEXTURE_1D;
        }
        case 2:
        {
            return GL_TEXTURE_2D;
        }
        case 3:
        {
            return GL_TEXTURE_3D;
        }
    }
    return GL_NONE; // Should be treated as invalid
}

bool bhDeviceGL::IsTextureValid(const bhTextureGL* texture)
{
    return ((glIsTexture(texture->id) == GL_TRUE) && (texture->target != GL_NONE));
}

bhTextureGL* bhDeviceGL::CreateTexture(const bhImage* img, GLint border, GLFormats glFmt)
{
    if (glFmt.fmt == GL_NONE)
    {
        return nullptr;
    }

    GLenum target = bhDeviceGL::MapImageTypeToOpenGL(bhImage_DetermineDimensions(img));
    if (target == GL_NONE)
    {
        return nullptr;
    }

    GLuint newID = GL_ZERO;
    glGenTextures(1, &newID);
    if (newID == GL_ZERO)
    {
        // Retrieve / report error?
        return nullptr;
    }
    glBindTexture(target, newID);
    switch (target)
    {
        case GL_TEXTURE_1D:
        {
            glTexImage1D(GL_TEXTURE_1D, 0, glFmt.internalFmt, img->width, border, glFmt.fmt, GL_UNSIGNED_BYTE, 0);
            break;
        }
        case GL_TEXTURE_2D:
        {
            glTexImage2D(GL_TEXTURE_2D, 0, glFmt.internalFmt, img->width, img->height, border, glFmt.fmt, GL_UNSIGNED_BYTE, 0);
            break;
        }
        case GL_TEXTURE_3D:
        {
            glTexImage3D(GL_TEXTURE_3D, 0, glFmt.internalFmt, img->width, img->height, img->depth, border, glFmt.fmt, GL_UNSIGNED_BYTE, 0);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const bhRenderSettings* rs = bhConfig_GetRenderSettings();
    GLint minFilter = (bhConfig_GenMipmaps(rs) != 0) ?
        GL_LINEAR_MIPMAP_LINEAR :
        GL_LINEAR;
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
    glBindTexture(target, 0);

    return new bhTextureGL(newID, target);
}

bhResourceID bhDeviceGL::CreateTextureFromImage(const bhImage* img, const char* name, int32_t offsetx, int32_t offsety)
{
    // Check if we already created the texture
    bhResourceID textureID = textureManager.Find(name);
    if (textureID)
    {
        return textureID;
    }

    // Try to make sense out of image components
    GLFormats glFmt = bhDeviceGL::PickImageFormat(img);
    bhTextureGL* texture = bhDeviceGL::CreateTexture(img, 0, glFmt);
    if (!texture)
    {
        return BH_INVALID_RESOURCE;
    }

    textureID = textureManager.AddNamedResource(name, texture);
    glBindTexture(texture->target, texture->id);
    switch (texture->target)
    {
        case GL_TEXTURE_1D:
        {
            glTexSubImage1D(GL_TEXTURE_1D, 0, GLint(offsetx), img->width, glFmt.fmt, GL_UNSIGNED_BYTE, img->pixels);
            break;
        }
        case GL_TEXTURE_2D:
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, GLint(offsetx), GLint(offsety), img->width, img->height, glFmt.fmt, GL_UNSIGNED_BYTE, img->pixels);
            break;
        }
        // Unusable cases are handled by DetermineTextureType() above
    }

    const bhRenderSettings* rs = bhConfig_GetRenderSettings();
    if ((bhConfig_GenMipmaps(rs) != 0) && bhImage_IsValidForMipmap(img))
    {
        glGenerateMipmap(texture->target);
    }
    glBindTexture(texture->target, GL_ZERO);
    return textureID;
}

void bhDeviceGL::GenerateMipmaps(bhResourceID textureID)
{
    bhTextureGL* texture = textureManager.GetResourceObject(textureID);
    glBindTexture(texture->target, texture->id);
    glGenerateMipmap(texture->target);
    glBindTexture(texture->target, GL_ZERO);
}

void bhDeviceGL::DestroyTexture(bhTextureGL* texture)
{
    glDeleteTextures(1, &(texture->id));
    texture = {};
}

GLFormats bhDeviceGL::PickImageFormat(const bhImage* img)
{
    GLFormats f;
    switch (img->numComponents)
    {
        case 1:
        {
            f.fmt = GL_RED;
            f.internalFmt = GL_COMPRESSED_RED;
            break;
        }
        case 2:
        {
            f.fmt = GL_RG;
            f.internalFmt = GL_COMPRESSED_RG;
            break;
        }
        case 3:
        {
            f.fmt = GL_RGB;
            f.internalFmt = GL_COMPRESSED_RGB;
            break;
        }
        case 4:
        {
            f.fmt = GL_RGBA;
            f.internalFmt = GL_COMPRESSED_RGBA;
            break;
        }
        default:
        {
            break;
        }
    }
    return f;
}

void bhDeviceGL::UseWindowFramebuffer()
{
    int w, h;
    bhGetMainWindowSize(&w, &h);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
}

bhBufferGL bhDeviceGL::CreateBuffer(GLenum target, GLenum usage, GLsizeiptr reqSize)
{
    bhBufferGL newBuffer;
    glGenBuffers(1, &(newBuffer.buffer));
    glBindBuffer(target, newBuffer.buffer);
    newBuffer.size = reqSize;
    glBufferData(target, newBuffer.size, nullptr, usage);
    return newBuffer;
}

void bhDeviceGL::DestroyBuffer(bhBufferGL* pBuffer)
{
    assert(pBuffer != nullptr);
    glDeleteBuffers(1, &(pBuffer->buffer));
    pBuffer->buffer = GL_ZERO;
    pBuffer->size = 0;
}

bool bhDeviceGL::CopyDataToBuffer(const bhBufferGL* buffer, GLintptr offset, GLsizei reqSize, const void* data)
{
    if (offset + reqSize > buffer->size)
    {
        bhLog_Message(LOG_TYPE_CRITICAL, "CopyDataToBuffer : Exceeding buffer size!");
        return false;
    }
    glNamedBufferSubData(buffer->buffer, offset, reqSize, data);
    return true;
}

#ifdef BH_USE_IMGUI

bool bhDeviceGL::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    if (ImGui_ImplSDL2_InitForOpenGL(bhGetMainWindow(), GLContext))
    {
        const bhRenderSettings* rs = bhConfig_GetRenderSettings();
        static const size_t VERSION_SRING_LEN = 16;
        char versionString[VERSION_SRING_LEN];
        sprintf_s(versionString, VERSION_SRING_LEN, "#version %d%d0", rs->gl.versionMajor, rs->gl.versionMinor);
        if (ImGui_ImplOpenGL3_Init(versionString))
        {
            ImGui::StyleColorsDark();
            return true;
        }
    }
    return false;
}

void bhDeviceGL::DestroyImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void bhDeviceGL::BeginImGuiFrame()
{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void bhDeviceGL::EndImGuiFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#endif //BH_USE_IMGUI
