#include "Private/bhResourceCache.hpp"
#include "GL/bhDeviceGL.hpp"
#include "bhImage.h"
#include "bhUtil.h"
#include "bhEnv.h"

namespace bhResources
{
    ////////////////////////////////////////////////////////////////////////////////
    static bhResourceCache<bhTextureGL, 8> textureCache;

    bhResourceID FindTextureByName(const std::string& name)
    {
        return textureCache.FindByName(name.c_str());
    }

    bhTextureGL* GetTextureObject(bhResourceID id)
    {
        return textureCache.Get(id);
    }

    //bhResourceID AddNamedMesh(const std::string& name, bhMesh* newMesh) { return textureCache.AddNamedResource(name, newMesh); }

    void DestroyTextures()
    {
        textureCache.DeleteAll([](bhTextureGL* texture) { static_cast<bhDeviceGL*>(bhGPUDevice::Get())->DestroyTexture(texture); });
    }

    bhResourceID GetTextureByName(const std::string& textureName)
    {
        bhResourceID texID = textureCache.FindByName(textureName.c_str());
        if (texID.IsValid())
        {
            return texID;
        }

        const char* path = bhUtil_CreatePath(bhEnv_GetEnvString(ENV_TEXTURES_PATH), textureName.c_str());
        bhImage img = bhImage_CreateFromFile(path, 4);
        if (img.pixels == nullptr)
        {
            bhLog_Message(LOG_TYPE_ERROR, "Could not load image %s", path);
            bhUtil_FreePath(&path);
            return {};
        }

        if (img.retCode == 1)
        {
            bhLog_Message(LOG_TYPE_WARNING, "Loading image %s: Runtime conversion", path);
        }
        bhUtil_FreePath(&path);

        bhDeviceGL* deviceGL = static_cast<bhDeviceGL*>(bhGPUDevice::Get());
        bhTextureGL* newTexture = deviceGL->CreateTextureFromImage(&img, 0, 0);
        bhImage_Delete(&img);
        if (newTexture)
        {
            texID = textureCache.New(textureName.c_str(), newTexture);
            assert(texID.IsValid());
            return texID;
        }
        return {};
    }
}
