#include "bhWindow.h"
#include "bhConfig.h"
#include "GL/bhConfigGL.h"
#include <assert.h>

namespace bhWindow
{
    ////////////////////////////////////////////////////////////////////////////////
#ifdef _glfw3_h_

    bool InitFramework()
    {
        PreInit();
        return (glfwInit() == GLFW_TRUE) && (glfwVulkanSupported() == GLFW_TRUE);
    }

    void SetupWindowHints()
    {
        bhConfig::Window const* ws = bhConfig::GetWindowSettings();
        bhConfigGL::OpenGL const* gls = bhConfigGL::GetOpenGLSettings();
        //bhConfig::System const* sys = bhConfig::GetSystemSettings();

        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gls->version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gls->version_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, gls->use_compat_profile ? GLFW_OPENGL_COMPAT_PROFILE : GLFW_OPENGL_CORE_PROFILE);

#ifdef BH_API_DEBUG_CONTEXT
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#endif

        if (ws->use_desktop_video_mode)
        {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            assert(monitor != nullptr);
            GLFWvidmode const* videoMode = glfwGetVideoMode(monitor);

            glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
        }
        else
        {
            glfwWindowHint(GLFW_RED_BITS, ws->red_bits);
            glfwWindowHint(GLFW_GREEN_BITS, ws->green_bits);
            glfwWindowHint(GLFW_BLUE_BITS, ws->blue_bits);
            glfwWindowHint(GLFW_REFRESH_RATE, ws->refresh_rate);
        }

        glfwWindowHint(GLFW_DEPTH_BITS, ws->depth_bits);
        glfwWindowHint(GLFW_STENCIL_BITS, ws->stencil_bits);
        glfwWindowHint(GLFW_SAMPLES, ws->aa_samples);
    }

#endif //_glfw3_h_

    ////////////////////////////////////////////////////////////////////////////////
#ifdef SDL_h_
    
    bool InitFramework()
    {
        return false;
    }

    void SetupWindowHints()
    {}

#endif //SDL_h_
}
