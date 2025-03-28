#ifndef BH_PLATFORM_HPP
#define BH_PLATFORM_HPP

namespace bhPlatform
{
  enum ResourceType
  {
    RT_TEXTURE,
    RT_SHADER_SRC,
    RT_SHADER_BIN_OPENGL,
    RT_SHADER_BIN_VULKAN,
    RT_PIPELINE,
    RT_MATERIAL,
    RT_MESH,
    //RT_CHARACTER,
    //RT_SOUND,
    RT_MAP,
    //RT_FONT,

    RT_NUM_RESOURCE_TYPES
  };

  bool Init();
  void Destroy();
  const char* GetDirSeparator();
  const char* CreateOpenFileDialog(bool getFullPath);
  const char* CreateSaveFileDialog(bool getFullPath);
  const char* GetExecutableDir();
  const char* CreateConfigFilePath(const char* configName);
  const char* GetDataDir();

  const char* GetResourceDir(ResourceType rt);
  const char* CreateResourcePath(ResourceType rt, const char* fileName, const char* fileExtension = nullptr);
  //const char* GetResourceDir(ResourceType rt);
  const char* GetFileExtension(ResourceType rt);
  //const char* CreatePath(ResourceType rt, const char* fileName);
  void FreePath(const char*& path);
}

#endif //BH_PLATFORM_H
