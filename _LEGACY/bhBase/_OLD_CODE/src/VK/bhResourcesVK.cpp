#include "Private/bhResourceCache.hpp"
#include "VK/bhDeviceVK.hpp"
#include "bhSystem.hpp"
#include "bhImage.hpp"
#include "VK/bhPipelineVK.hpp"
#include "bhUtil.hpp"
#include "bhEnv.hpp"
#include "bhLog.hpp"

namespace bhResourcesVK
{
    class bhTextureResource : public bhResource
    {
    public:
        bhTextureResource() = default;
        bhTextureResource(bhTextureVK* tx)
        {
            texture = std::move(*tx);
        }

        bhTextureVK* GetTexture()
        {
            return &texture;
        }

        void DestroyResource() override final
        {
            bhSystem::GetRenderDevice()->DestroyTexture(texture);
        }

    protected:
    private:
        bhTextureVK texture;
    };

    static bhResourceCache<bhTextureResource, 8> textureCache;

    bhResourceID FindTextureByName(const std::string& name)
    {
        return textureCache.FindByName(name.c_str());
    }

    bhTextureVK* GetTextureObject(bhResourceID id)
    {
        return textureCache.Get(id)->GetTexture();
    }

    //bhResourceID AddNamedMesh(const std::string& name, bhMesh* newMesh) { return textureCache.AddNamedResource(name, newMesh); }

    void DestroyTextures()
    {
        textureCache.DeleteAll([](bhTextureResource& textureRsrc) { textureRsrc.DestroyResource(); });
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

        const bhDeviceVK* device = bhSystem::GetRenderDevice();
        bhTextureResource newTextureRsrc(device->CreateTextureFromImage(&img, 0, 0));

        texID = textureCache.New(textureName.c_str(), newTextureRsrc);
        assert(texID.IsValid());
        return texID;
    }

    ////////////////////////////////////////////////////////////////////////////////
    class bhWorldMaterialResource : public bhResource
    {
    public:
        bhWorldMaterialResource() = default;
        bhWorldMaterialResource(bhWorldMaterialVK* mat)
        {
            material = std::move(*mat);
        }

        bhWorldMaterialVK* GetMaterial()
        {
            return &material;
        }

        void DestroyResource(const bhWorldPipelineVK* pipeline) override final
        {
            pipeline->DestroyMaterial(this);
        }

    protected:
    private:
        bhWorldMaterialVK material;
    };

    static bhResourceCache<bhWorldMaterialResource, 8> worldMatCache;

}
