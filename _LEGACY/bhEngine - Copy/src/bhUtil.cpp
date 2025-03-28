#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "bhUtil.hpp"
#include "bhLog.h"

// See ref. https://graphics.stanford.edu/~seander/bithacks.html

namespace bhUtil
{
  const char* GetFileExtension(char const* fileName)
  {
    long long lastPos = strlen(fileName);
    if (lastPos > 0)
    {
      --lastPos; // Last string char
      while (lastPos > -1)
      {
        if (fileName[lastPos] == '.')
        {
          return &(fileName[lastPos + 1]);
        }
        --lastPos;
      }
    }
    return nullptr;
  }

  bool IsFileType(const char* fileName, const char** extensions)
  {
    const char* ext = GetFileExtension(fileName);
    while (*extensions)
    {
      if (strcmp(ext, *extensions) == 0) return true;
      ++extensions;
    }
    return false;
  }

  long GetFileSize(FILE* file)
  {
    fpos_t currPos = 0;
    fgetpos(file, &currPos); // in case we do a GetFileSize
    fseek(file, 0, SEEK_END);
    long sz = ftell(file);
    fsetpos(file, &currPos);
    return sz;
  }

  long ReadFile(const char* filePath, bool binary, void*& fd)
  {
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, filePath, binary ? "rb" : "r");
    if (err)
    {
      // TODO: Report error
      bhLog_Message(LP_ERROR, "Could not open file %s for reading", filePath);
      return {};
    }
    assert(file); // If the above check passes, this should never fail

    long length = GetFileSize(file);
    if (!binary)
    {
      ++length;
    }
    fd = (void*)calloc(length, sizeof(unsigned char));
    size_t retCode = fread(fd, sizeof(unsigned char), length, file);
    if (retCode != length)
    {
      FreeFileData(fd);
      return 0;
    }
    fclose(file);
    return length;
  }

  void FreeFileData(void*& fd)
  {
    free(fd);
    fd = nullptr;
  }

  void* AlignedAlloc(size_t size, size_t alignment)
  {
    void* data = NULL;
  #if defined(_MSC_VER) || defined(__MINGW32__)
    data = _aligned_malloc(size, alignment);
  #else
    int res = posix_memalign(&data, alignment, size);
    if (res != 0)
      data = nullptr;
  #endif
    return data;
  }

  void AlignedFree(void* data)
  {
  #if	defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(data);
  #else
    free(data);
  #endif
  }

  //uint8_t CountBits(uint32_t val)
  //{
  //	uint8_t numBits = 0;
  //	while (val > 0)
  //	{
  //	    if (val & 1)
  //	    {
  //	        ++numBits;
  //	    }
  //	    val = val >> 1;
  //	}
  //	return numBits;
  //}

  //int CheckFileExists(const char* path)
  //{
  //	struct stat fileInfo = {};
  //	return stat(path, &fileInfo);
  //}

  //int CheckDirectoryExists(const char* path)
  //{
  //	struct stat fileInfo = {};
  //	if (stat(path, &fileInfo) == 0) // Success
  //	{
  //		return (int)!(fileInfo.st_mode & S_IFDIR);
  //	}
  //	return 1;
  //}
}
