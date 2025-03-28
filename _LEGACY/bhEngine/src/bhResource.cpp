//#include <dirent.h>
#include "bhResource.hpp"
#include "bhUtil.hpp"
#include "Platform/bhPlatform.hpp"

const std::string INVALID_RESOURCE_PATH_STR("INVALID_RESOURCE_PATH");

bhResourceCache<uint32_t> tCache;

#ifdef DIRENT_H

void GenerateResourceHashes(std::unordered_map<bhHash_t, bhResourceInfo>& hashes, const char* dir, const char** extensions, const char* subDir)
{
  static char currDir[BH_PATH_BUF_LEN];

  sprintf_s(currDir, BH_PATH_BUF_LEN, "%s%s", dir, subDir ? subDir : "");
  DIR* baseDir = opendir(currDir);
  if (baseDir)
  {
    while (dirent* entry = readdir(baseDir))
    {
      switch (entry->d_type)
      {
        case DT_REG: // A file
        {
          static char fileName[BH_PATH_BUF_LEN];
          if (!bhUtil::IsFileType(entry->d_name, extensions)) continue;
          sprintf_s(fileName, BH_PATH_BUF_LEN, "%s%s", subDir ? subDir : "", entry->d_name);
          hashes.emplace(bhHash(fileName), bhResourceInfo(entry->d_name, fileName));
          continue;
        }
        case DT_DIR: // A directory
        {
          if (strcmp(entry->d_name, ".") == 0) continue;
          if (strcmp(entry->d_name, "..") == 0) continue;

          static char nextSubDir[BH_PATH_BUF_LEN];
          sprintf_s(nextSubDir, BH_PATH_BUF_LEN, "%s%s%s%s", subDir ? subDir : "", subDir ? bhPlatform::GetDirSeparator() : "", entry->d_name, bhPlatform::GetDirSeparator());
          GenerateResourceHashes(hashes, dir, extensions, nextSubDir);
          break;
        }
        case DT_LNK: // Etc.
        default:
        {
          continue;
        }
      }
    }
    closedir(baseDir);
  }
}

#endif //DIRENT_H
