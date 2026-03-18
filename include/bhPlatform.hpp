#ifndef BH_PLATFORM_HPP
#define BH_PLATFORM_HPP

namespace bhPlatform
{
  constexpr const char* BH_DIR_SEPARATOR =
#if SDL_PLATFORM_WINDOWS
    "\\";
#else
    "/";
#endif

  enum class ResourceType
  {
    RT_IMAGE,
    //RT_SHADER_SRC,
    RT_SHADER,
    //RT_PIPELINE,
    //RT_MATERIAL,
    RT_MESH,
    //RT_CHARACTER,
    //RT_SOUND,
    RT_MAP,
    //RT_FONT,

    RT_NUM_RESOURCE_TYPES
  };

  bool Init();
  void Destroy();
  //const char* CreateOpenFileDialog(bool getFullPath);
  //const char* CreateSaveFileDialog(bool getFullPath);
  const char* GetExecutableDir();
  const char* CreateConfigFilePath(const char* configName);
  bool SetDataDir(const char* path);
  const char* GetDataDir();

  const char* GetResourceDir(ResourceType rt);
  const char* CreateResourcePath(ResourceType rt, const char* fileName); //Returned value must be freed by caller
  //const char* CreatePath(ResourceType rt, const char* fileName);
  void FreePath(const char*& path);

  bool CheckDirectoryExists(const char* path);
}

#endif //BH_PLATFORM_HPP
