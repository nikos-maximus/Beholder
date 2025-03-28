#ifndef BH_UTIL_HPP
#define BH_UTIL_HPP

namespace bhUtil
{
  template<typename Obj_t>
  inline void Swap(Obj_t& o0, Obj_t& o1)
  {
    Obj_t tmp = o0;
    o0 = o1;
    o1 = tmp;
  }

  template<typename Scalar_t>
  inline Scalar_t Min(Scalar_t a, Scalar_t b)
  {
    if (a < b)
      return a;
    return b;
  }

  template<typename Scalar_t>
  inline Scalar_t Max(Scalar_t a, Scalar_t b)
  {
    if (a > b)
      return a;
    return b;
  }

  const char* GetFileExtension(char const* fileName);
  // extensions is a null-terminated array of c-strings
  // containing file extensions without the '.' e.g. { "jpg", "png" }
  bool IsFileType(const char* fileName, const char** extensions);
  long ReadFile(const char* filePath, bool binary, void*& fd);
  void FreeFileData(void*& fd);
  void* AlignedAlloc(size_t size, size_t alignment);
  void AlignedFree(void* data);

  //uint8_t CountBits(uint32_t val);
  //int bhUtil_CheckFileExists(const char* path); // 0 on success
  //int bhUtil_CheckDirectoryExists(const char* path); // 0 on success
}

#endif //BH_UTIL_HPP
