#ifndef BH_IMAGE_HPP
#define BH_IMAGE_HPP

#include <stdint.h>

class bhImage
{
public:
  static void InitEnv();
  static bhImage* CreateEmpty(int w, int h, int numComponents);
  static bhImage* CreateFromFile(const char* filePath, int reqComponents); // See definition of STBI_default -> stb_image.h

  int DetermineDimensions() const;
  bool IsValidForMipmap() const;
  uint32_t GetRequiredMipLevels() const;
  void Free();
  int PutPixel(int x, int y, uint32_t color);
  int SavePPM_Text(const char* path);

  __forceinline size_t GetPixelMemSiz() const
  {
    return sizeof(uint8_t) * numComponents;
  }

  __forceinline size_t GetMemSiz() const
  {
    return size_t(width) * size_t(height) * size_t(depth) * GetPixelMemSiz();
  }

  __forceinline bool ArePixelCoordsValid(int x, int y) const
  {
    return ((0 <= x) && (x < width)) && ((0 <= y) && (y < height));
  }

protected:
private:
  uint8_t* pixels{ nullptr }; // Muat match stbi_uc def --> unsigned char
  int width{ 0 }, height{ 0 }, depth{ 0 };
  int numComponents{ 0 };
};

#endif //BH_IMAGE_HPP
