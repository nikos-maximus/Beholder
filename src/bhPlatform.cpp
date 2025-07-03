#include <stdio.h>
#include <SDL3/SDL.h>
#include "bhPlatform.hpp"
#include "bhUtil.hpp"

#define DATA_DIR_NAME "Data"

namespace bhPlatform
{
  static char* g_dataDir = nullptr;
  static char* g_resourceDirs[static_cast<uint8_t>(ResourceType::RT_NUM_RESOURCE_TYPES)] = {};

  void Destroy()
  {
    constexpr uint8_t INT_NUM_RT = static_cast<uint8_t>(ResourceType::RT_NUM_RESOURCE_TYPES);
    for (uint8_t intRT = 0; intRT < INT_NUM_RT; ++intRT)
    {
      if (g_resourceDirs[intRT])
      {
        delete[] g_resourceDirs[intRT];
        g_resourceDirs[intRT] = nullptr;
      }
    }
    delete[] g_dataDir;
    g_dataDir = nullptr;
  }

  const char* GetExecutableDir()
  {
    return SDL_GetBasePath();
  }

  constexpr const char* GetDirSeparator()
  {
  #if SDL_PLATFORM_WINDOWS
    return "\\";
  #else
    return "/";
  #endif
  }

  const char* GetResourceTypeStr(ResourceType rt)
  {
    uint8_t intRT = static_cast<uint8_t>(rt);
    constexpr uint8_t INT_NUM_RT = static_cast<uint8_t>(ResourceType::RT_NUM_RESOURCE_TYPES);
    static const char* rtStrings[INT_NUM_RT] =
    {
        "Images",
        //"Shaderssrc",
        //"Shadersgl",
        //"Shadersvk",
        //"Pipelines",
        //"Materials",
        //"Meshes",
        //"Characters",
        //"Sounds",
        "Maps",
        //"Fonts"
    };

    if ((0 <= intRT) && (intRT < INT_NUM_RT))
    {
      return rtStrings[intRT];
    }
    return nullptr;
  }

  void FreePath(const char*& path)
  {
    delete[] path;
  }

  const char* CreateConfigFilePath(const char* configName)
  {
    size_t len = strlen(GetExecutableDir()) + strlen(GetDirSeparator()) + strlen(configName) + 1;
    char* cfp = new char[len];
    sprintf_s(cfp, len, "%s%s%s", GetExecutableDir(), GetDirSeparator(), configName);
    return cfp;
  }

  bool SetDataDir(const char* path)
  {
    bool needsPostfix = !bhUtil::EndsWith(path, GetDirSeparator());
    size_t len = strlen(path) + (needsPostfix ? strlen(GetDirSeparator()) : 0) + 1;
    char* newDataDir = new char[len];
    sprintf_s(newDataDir, len, "%s%s", path, needsPostfix ? GetDirSeparator() : "");
    if (CheckDirectoryExists(newDataDir))
    {
      delete[] g_dataDir;
      g_dataDir = new char[len];
      strncpy_s(g_dataDir, len, newDataDir, len);
      delete[] newDataDir;
      return true;
    }
    delete[] newDataDir;
    return false;
  }

  const char* GetDataDir()
  {
    if (!g_dataDir)
    {
      size_t len = strlen(GetExecutableDir()) + strlen(DATA_DIR_NAME) + strlen(GetDirSeparator()) + 1;
      g_dataDir = new char[len];
      sprintf_s(g_dataDir, len, "%s%s%s", GetExecutableDir(), DATA_DIR_NAME, GetDirSeparator());
    }
    return g_dataDir;
  }

  const char* GetResourceDir(ResourceType rt)
  {
    uint8_t intRT = static_cast<uint8_t>(rt);
    constexpr uint8_t INT_NUM_RT = static_cast<uint8_t>(ResourceType::RT_NUM_RESOURCE_TYPES);

    if (intRT < INT_NUM_RT)
    {
      if (g_resourceDirs[intRT] == nullptr)
      {
        size_t len = strlen(GetDataDir()) + strlen(GetResourceTypeStr(rt)) + strlen(GetDirSeparator()) + 1;
        g_resourceDirs[intRT] = new char[len];
        sprintf_s(g_resourceDirs[intRT], len, "%s%s%s", GetDataDir(), GetResourceTypeStr(rt), GetDirSeparator());
      }
      return g_resourceDirs[intRT];
    }
    return nullptr;
  }

  const char* CreateResourcePath(ResourceType rt, const char* fileName)
  {
    const char* resourceDir = GetResourceDir(rt);
    size_t len = strlen(resourceDir) + strlen(fileName) + 1;
    char* newPath = new char[len];
    sprintf_s(newPath, len, "%s%s", resourceDir, fileName);
    return newPath;
  }

  bool CheckDirectoryExists(const char* path)
  {
    struct stat fileInfo = {};
    if (stat(path, &fileInfo) == 0) // Success
    {
      return bool(fileInfo.st_mode & S_IFDIR);
    }
    return false;
  }
}

#if _OLD_
const char* Lpwstr2Str(const LPWSTR inStr)
{
  size_t len = wcslen(inStr) + 1; //Need the trailing \0
  char* outStr = new char[len];
  int res = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, inStr, -1, outStr, int(len), NULL, NULL);
  if (res > 0)
  {
    return outStr;
  }
  //Chance to log error?
  return nullptr;
}

void FreePathW(const WCHAR*& path)
{
  delete[] path;
}

bool Init()
{
  HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hStdOut == INVALID_HANDLE_VALUE)
  {
    //GetLastError();
    return false;
  }

  DWORD dMode = 0;
  if (!GetConsoleMode(hStdOut, &dMode))
  {
    //GetLastError();
    return false;
  }
  if (!(dMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING))
  {
    dMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hStdOut, dMode))
    {
      //GetLastError();
      return false;
    }
  }
  GetExecutableDir();
  GetDataDir();
  return true;
}

const WCHAR* Str2Lpwstr(const char* inStr)
{
  size_t len = strlen(inStr) + 1;
  WCHAR* outStr = new WCHAR[len];

  int wchars_num = MultiByteToWideChar(CP_UTF8, 0, inStr, -1, NULL, 0);
  if (wchars_num <= 0)
  {
    return nullptr;
  }
  wchars_num = MultiByteToWideChar(CP_UTF8, 0, inStr, -1, outStr, wchars_num);
  return outStr;
}

const char* CreateNativeFileDialog(bool abSave, bool getFullPath)
{
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (SUCCEEDED(hr))
  {
    IFileDialog* pFileDialog;

    // Create the FileOpenDialog object.
    const CLSID clsid = abSave ? CLSID_FileSaveDialog : CLSID_FileOpenDialog;
    const IID iid = abSave ? IID_IFileSaveDialog : IID_IFileOpenDialog;
    hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL, iid, reinterpret_cast<void**>(&pFileDialog));

    if (SUCCEEDED(hr))
    {
      // Show the Open dialog box.
      hr = pFileDialog->Show(NULL);

      // Get the file name from the dialog box.
      if (SUCCEEDED(hr))
      {
        IShellItem* pItem;
        hr = pFileDialog->GetResult(&pItem);
        if (SUCCEEDED(hr))
        {
          PWSTR pszFilePath;
          SIGDN displayFlag = getFullPath ? SIGDN_FILESYSPATH : SIGDN_NORMALDISPLAY;
          hr = pItem->GetDisplayName(displayFlag, &pszFilePath);

          // Display the file name to the user.
          if (SUCCEEDED(hr))
          {
            const char* pathBuf = Lpwstr2Str(pszFilePath);

            //(NULL, pszFilePath, L"File Path", MB_OK); //???
            CoTaskMemFree(pszFilePath);

            return pathBuf;
          }
          pItem->Release();
        }
      }
      pFileDialog->Release();
    }
    CoUninitialize();
  }
  return nullptr;
}

const char* CreateOpenFileDialog(bool getFullPath)
{
  return CreateNativeFileDialog(false, getFullPath);
}

const char* CreateSaveFileDialog(bool getFullPath)
{
  return CreateNativeFileDialog(true, getFullPath);
}

const char* CreatePath(ResourceType rt, const char* fileName)
{
    const char* dir = GetResourceTypeStr(rt);
    size_t pathLen = strlen(dir) + strlen(fileName) + 2; // + path delim + null-terminate
    char* path = new char[pathLen];
    sprintf_s(path, pathLen, "%s%c%s", dir, PATH_DELIM, fileName);
    return path;
}

const char* GetExecutableDir()
{
  if (g_executablePath)
  {
    return g_executablePath;
  }
  HMODULE hModule = GetModuleHandle(NULL);
  if (hModule == NULL)
  {
    //GetLastError(); //??
    return nullptr;
  }
  WCHAR path[BH_PATH_BUF_LEN];
  DWORD nChars = GetModuleFileName(hModule, path, BH_PATH_BUF_LEN);
  PathRemoveFileSpec(path);
  if (nChars < 1)
  {
    return nullptr;
  }
  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    //Path was truncated: buffer was too small
  }
  g_executablePath = Lpwstr2Str(path);
  return g_executablePath;
}
#endif //_OLD_
