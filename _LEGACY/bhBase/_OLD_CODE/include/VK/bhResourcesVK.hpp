#ifndef BH_RESOURCES_VK_HPP
#define BH_RESOURCES_VK_HPP

#include "Private/bhResource.hpp"
#include "VK/bhPipelineVK.hpp"

namespace bhResourcesVK
{
    ////////////////////////////////////////////////////////////////////////////////
    bhResourceID FindTextureByName(const std::string& name);
    bhTextureVK* GetTextureObject(bhResourceID id);
    //bhResourceID AddNamedMesh(const std::string& name, bhMesh* newMesh) { return textureCache.AddNamedResource(name, newMesh); }
    void DestroyTextures();
    bhResourceID GetTextureByName(const std::string& textureName);
}

#endif //BH_RESOURCES_VK_HPP
