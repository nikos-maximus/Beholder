#include <SDL3/SDL.h>
//#include <vector>
//#include <Windows.h>
//#include <ShObjIdl.h>
//#include <Shlwapi.h>
//#include "bhDefines.h"
#include "bhPlatform.hpp"

namespace bhPlatform
{
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

#if _OLD
  static const char* g_executablePath = nullptr;
  static char* g_dataPath = nullptr;

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

  const char* GetResourceDirStr(ResourceType rt)
  {
    static const char* envStrings[RT_NUM_RESOURCE_TYPES] =
    {
        "\\Textures\\",
        "\\Shaders\\src\\",
        "\\Shaders\\gl\\",
        "\\Shaders\\vk\\",
        "\\Pipelines\\",
        "\\Materials\\",
        "\\Meshes\\",
        //"Characters",
        //"Sounds",
        "\\Maps\\",
        //"Fonts"
    };
    if ((0 <= rt) && (rt < RT_NUM_RESOURCE_TYPES))
    {
      return envStrings[rt];
    }
    return nullptr;
  }

  const char* GetFileExtension(ResourceType rt)
  {
    switch (rt)
    {
      case RT_MESH:
      {
        return "bms";
      }
      default:
      {
        return nullptr;
      }
    }
  }

  //const char* CreatePath(ResourceType rt, const char* fileName)
  //{
  //    const char* dir = GetResourceDirStr(rt);
  //    size_t pathLen = strlen(dir) + strlen(fileName) + 2; // + path delim + null-terminate
  //    char* path = new char[pathLen];
  //    sprintf_s(path, pathLen, "%s%c%s", dir, PATH_DELIM, fileName);
  //    return path;
  //}

  void FreePath(const char*& path)
  {
    delete[] path;
  }

  const char* CreateConfigFilePath(const char* configName)
  {
    size_t len = strlen(g_executablePath) + strlen(GetDirSeparator()) + strlen(configName) + 1;
    char* cfp = new char[len];
    sprintf_s(cfp, len, "%s%s%s", g_executablePath, GetDirSeparator(), configName);
    return cfp;
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

  const char* GetDataDirName()
  {
    return "Data";
  }

  const char* GetDataDir()
  {
    if (!g_dataPath)
    {
      size_t len = strlen(g_executablePath) + strlen(GetDirSeparator()) + strlen(GetDataDirName()) + 1;
      g_dataPath = new char[len];
      sprintf_s(g_dataPath, len, "%s%s%s", g_executablePath, GetDirSeparator(), GetDataDirName());
    }
    return g_dataPath;
  }

  const char* GetResourceDir(ResourceType rt)
  {
    size_t len = strlen(g_dataPath) + strlen(GetResourceDirStr(rt)) + 1;
    char* newPath = new char[len];
    sprintf_s(newPath, len, "%s%s", g_dataPath, GetResourceDirStr(rt));
    return newPath;
  }

  const char* CreateResourcePath(ResourceType rt, const char* fileName, const char* fileExtension)
  {
    size_t len = strlen(g_dataPath) + strlen(GetResourceDirStr(rt)) + strlen(fileName) + 1;
    if (fileExtension)
    {
      len += 1 + strlen(fileExtension); //Plus the '.'
    }
    char* newPath = new char[len];
    if (fileExtension)
    {
      sprintf_s(newPath, len, "%s%s%s.%s", g_dataPath, GetResourceDirStr(rt), fileName, fileExtension);
    }
    else
    {
      sprintf_s(newPath, len, "%s%s%s", g_dataPath, GetResourceDirStr(rt), fileName);
    }
    return newPath;
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

  void Destroy()
  {
    delete[] g_executablePath;
    g_executablePath = nullptr;

    delete[] g_dataPath;
    g_dataPath = nullptr;
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
#endif //_OLD
}
