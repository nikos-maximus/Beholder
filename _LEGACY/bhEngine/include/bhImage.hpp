#ifndef BH_IMAGE_H
#define BH_IMAGE_H

#include <stdint.h>

struct bhImage
{
  uint8_t* pixels{ nullptr }; // Muat match stbi_uc def --> unsigned char
  int width{ 0 }, height{ 0 }, depth{ 0 };
  int numComponents{ 0 };

  static void InitEnv();
  static int DetermineDimensions(const bhImage& img);
  static bhImage* CreateEmpty(int w, int h, int numComponents);
  static bhImage* CreateFromFile(const char* filePath, int reqComponents); // See definition of STBI_default -> stb_image.h
  static int IsValidForMipmap(const bhImage& img);
  static uint32_t GetRequiredMipLevels(const bhImage& img);
  static void Free(bhImage* img);
  static int PutPixel(const bhImage& img, int x, int y, uint32_t color);
  static int SavePPM_Text(const bhImage& img, const char* path);

  __forceinline static size_t GetPixelMemSiz(const bhImage& img)
  {
    return sizeof(uint8_t) * img.numComponents;
  }

  __forceinline static size_t GetMemSiz(const bhImage& img)
  {
    return size_t(img.width) * size_t(img.height) * size_t(img.depth) * GetPixelMemSiz(img);
  }
};

#endif //BH_IMAGE_H
