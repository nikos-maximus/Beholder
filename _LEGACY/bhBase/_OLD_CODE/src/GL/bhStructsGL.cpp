#include "GL/bhStructsGL.hpp"
#include "bhLog.h"
#include "bhUtil.h"
#include "bhConfig.h"
#include "bhEnv.h"
#include "bhMesh.hpp"

////////////////////////////////////////////////////////////////////////////////
#ifdef BH_API_DEBUG

#if 0 // Old glGetError Functionality
GLenum bhGL_CheckError(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        const char* errorStr = nullptr;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
            {
                errorStr = "INVALID_ENUM";
                break;
            }
            case GL_INVALID_VALUE:
            {
                errorStr = "INVALID_VALUE";
                break;
            }
            case GL_INVALID_OPERATION:
            {
                errorStr = "INVALID_OPERATION";
                break;
            }
            case GL_STACK_OVERFLOW:
            {
                errorStr = "STACK_OVERFLOW";
                break;
            }
            case GL_STACK_UNDERFLOW:
            {
                errorStr = "STACK_UNDERFLOW";
                break;
            }
            case GL_OUT_OF_MEMORY:
            {
                errorStr = "OUT_OF_MEMORY";
                break;
            }
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                errorStr = "INVALID_FRAMEBUFFER_OPERATION";
                break;
            }
            //case GL_CONTEXT_LOST:
        }
    }
    return errorCode;
}
#endif // Old glGetError Functionality

void APIENTRY glDebugOutput(
    GLenum source, 
    GLenum type, 
    unsigned int id, 
    GLenum severity, 
    GLsizei length, 
    const char* message, 
    const void* userParam)
{
    // ignore non-significant error/warning codes
    //if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    const char* sourceStr = nullptr;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
        {
            sourceStr = "API";
            break;
        }
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        {
            sourceStr = "Window System";
            break;
        }
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        {
            sourceStr = "Shader Compiler";
            break;
        }
        // The following sources are only used for application-generated messages
        // See glDebugMessageInsert
        //case GL_DEBUG_SOURCE_THIRD_PARTY:
        //{
        //    sourceStr = "Third Party";
        //    break;
        //}
        //case GL_DEBUG_SOURCE_APPLICATION:
        //{
        //    sourceStr = "Application";
        //    break;
        //}
        case GL_DEBUG_SOURCE_OTHER:
        {
            sourceStr = "Other";
            break;
        }
    }

    const char* typeStr = nullptr;
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
        {
            typeStr = "Error";
            break;
        }
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        {
            typeStr = "Deprecated Behaviour";
            break;
        }
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        {
            typeStr = "Undefined Behaviour";
            break;
        }
        case GL_DEBUG_TYPE_PORTABILITY:
        {
            typeStr = "Portability";
            break;
        }
        case GL_DEBUG_TYPE_PERFORMANCE:
        {
            typeStr = "Performance";
            break;
        }
        case GL_DEBUG_TYPE_MARKER:
        {
            typeStr = "Marker";
            break;
        }
        case GL_DEBUG_TYPE_PUSH_GROUP:
        {
            typeStr = "Push Group";
            break;
        }
        case GL_DEBUG_TYPE_POP_GROUP:
        {
            typeStr = "Pop Group";
            break;
        }
        case GL_DEBUG_TYPE_OTHER:
        {
            typeStr = "Other";
            break;
        }
    }

    bhLogType severityLogType = bhLogType::LOG_NUM_TYPES;
    const char* severityStr = nullptr;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        {
            severityStr = "High";
            severityLogType = bhLogType::LOG_TYPE_CRITICAL;
            break;
        }
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
            severityStr = "Medium";
            severityLogType = bhLogType::LOG_TYPE_ERROR;
            break;
        }
        case GL_DEBUG_SEVERITY_LOW:
        {
            severityStr = "Low";
            severityLogType = bhLogType::LOG_TYPE_WARNING;
            break;
        }
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        {
            severityStr = "Notification";
            severityLogType = bhLogType::LOG_TYPE_ERROR;
            break;
        }
    }

    static char glDebugMsgBuf[BH_LOG_MESSAGE_SIZ];
    sprintf_s(glDebugMsgBuf, BH_LOG_MESSAGE_SIZ, "GL message (id: %d): %s - Source: %s | Type: %s | Severity: %s\n", id, message, sourceStr, typeStr, severityStr);
    bhLog_Message(severityLogType, "%s", glDebugMsgBuf);
}

GLboolean bhGL_InitDebugCallback()
{
    // See also: https://www.khronos.org/opengl/wiki/Debug_Output
    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    return GL_TRUE;
}

void bhGL_DestroyDebugCallback()
{
    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glDebugMessageCallback(nullptr, nullptr);
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDisable(GL_DEBUG_OUTPUT);
    }
}

#endif //BH_API_DEBUG

////////////////////////////////////////////////////////////////////////////////
bool CreateShaderFromGLSL(bhShaderType shaderType, const FileReadData& fileData, GLuint& newShader)
{
    newShader = glCreateShader(bhGL_GetShaderType(shaderType));
    if (newShader)
    {
        glShaderSource(newShader, 1, &(fileData.data), 0); // That will work for only one string with full source
        glCompileShader(newShader);
        GLint status = GL_FALSE;
        glGetShaderiv(newShader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint infoLogLength;
            glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &infoLogLength);

            std::vector<GLchar> infoLog(infoLogLength + 1);
            glGetShaderInfoLog(newShader, infoLogLength, NULL, infoLog.data());
            bhLog_Message(LOG_TYPE_ERROR, "While compiling shader - Infolog : %s", infoLog.data());
            glDeleteShader(newShader);
            return false;
        }
    }
    return (newShader > 0);
}

bool CreateShaderFromSPIRV(bhShaderType shaderType, const FileReadData& fileData, GLuint& newShader)
{
    newShader = glCreateShader(bhGL_GetShaderType(shaderType));
    if (newShader)
    {
        glShaderBinary(1, &newShader, GL_SHADER_BINARY_FORMAT_SPIR_V, fileData.data, fileData.length);
        glSpecializeShader(newShader, "main", 0, nullptr, nullptr);
        GLint status = GL_FALSE;
        glGetShaderiv(newShader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint infoLogLength;
            glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &infoLogLength);

            std::vector<GLchar> infoLog(infoLogLength + 1);
            glGetShaderInfoLog(newShader, infoLogLength, NULL, infoLog.data());
            bhLog_Message(LOG_TYPE_ERROR, "While compiling shader - Infolog : %s", infoLog.data());
            glDeleteShader(newShader);
            return false;
        }
    }
    return (newShader > 0);
}

const char* GetShadersPath()
{
    const bhRenderSettings* rs = bhConfig_GetRenderSettings();
    return bhEnv_GetEnvString((bhConfig_GL_UseCompiledShaders(rs) != 0) ? ENV_SHADERS_BIN_OPENGL_PATH : ENV_SHADERS_SRC_PATH);
}

bhShaderType bhGL_CreateShaderFromFile(const char* fileName, GLuint& newShader)
{
    char* path = bhUtil_CreatePath(GetShadersPath(), fileName);
    bhShaderType type = bhTypes_GetShaderTypeFromFileExtension(path);

    if (type == SHADER_TYPE_UNKNOWN)
    {
        return SHADER_TYPE_UNKNOWN;
    }

    const bhRenderSettings* rs = bhConfig_GetRenderSettings();
    int binary = bhConfig_GL_UseCompiledShaders(rs) != 0;
    FileReadData fileData = bhUtil_ReadFile(path, binary);
    bhUtil_FreePath(&path);
    
    if (!fileData.data)
    {
        bhLog_Message(LOG_TYPE_ERROR, "Could not read shader file %s", path);
        return SHADER_TYPE_UNKNOWN;
    }

    if (binary)
    {
        if (CreateShaderFromSPIRV(type, fileData, newShader))
        {
            bhUtil_FreeFileData(&fileData);
            return type;
        }
    }
    else
    {
        if (CreateShaderFromGLSL(type, fileData, newShader))
        {
            bhUtil_FreeFileData(&fileData);
            return type;
        }
    }
    return SHADER_TYPE_UNKNOWN;
}

GLenum bhGL_GetShaderType(bhShaderType type)
{
    GLenum GL_ShaderTypes[NUM_SHADER_TYPES] =
    {
        GL_VERTEX_SHADER,
        GL_TESS_CONTROL_SHADER,
        GL_TESS_EVALUATION_SHADER,
        GL_GEOMETRY_SHADER,
        GL_FRAGMENT_SHADER,
        GL_COMPUTE_SHADER
    };
    return GL_ShaderTypes[(int)type];
}

GLuint bhGL_CreateProgramFromShaders(const std::vector<std::string>* shaderNames_v)
{
    GLuint programID = glCreateProgram();
    if(programID == 0)
    {
        assert(false);
        return 0;
    }

    std::vector<GLuint> shaders_v(shaderNames_v->size());
    for (size_t sh = 0; sh < shaders_v.size(); ++sh)
    {
        if (bhGL_CreateShaderFromFile((*shaderNames_v)[sh].c_str(), shaders_v[sh]) != SHADER_TYPE_UNKNOWN)
        {
            glAttachShader(programID, shaders_v[sh]);
        }
    }
    glLinkProgram(programID);

    for (size_t sh = 0; sh < shaders_v.size(); ++sh)
    {
        glDetachShader(programID, shaders_v[sh]);
        glDeleteShader(shaders_v[sh]);
    }

    GLint status = GL_FALSE;
    glGetProgramiv(programID, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* infoLog = static_cast<GLchar*>(calloc(infoLogLength + 1, sizeof(GLchar)));
        glGetProgramInfoLog(programID, infoLogLength, NULL, infoLog);
        bhLog_Message(LOG_TYPE_ERROR, "Program link failed: %s", infoLog);
        free(infoLog);

        glDeleteProgram(programID);
        return 0;
    }
    return programID;
}
