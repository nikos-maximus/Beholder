#ifndef BH_RESOURCES_GL_HPP
#define BH_RESOURCES_GL_HPP

#include "Private/bhResource.hpp"
#include "GL/bhPipelineGL.hpp"

namespace bhResources
{
    ////////////////////////////////////////////////////////////////////////////////
    bhResourceID FindTextureByName(const std::string& name);
    bhTextureGL* GetTextureObject(bhResourceID id);
    //bhResourceID AddNamedMesh(const std::string& name, bhMesh* newMesh) { return textureCache.AddNamedResource(name, newMesh); }
    void DestroyTextures();
    bhResourceID GetTextureByName(const std::string& textureName);
}

#endif //BH_RESOURCES_GL_HPP
