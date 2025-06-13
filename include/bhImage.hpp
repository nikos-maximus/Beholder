#ifndef BH_IMAGE_HPP
#define BH_IMAGE_HPP

#include <stdint.h>

class bhImage
{
public:
  static void InitEnv();
  static bhImage* CreateEmpty(int w, int h, int numComponents);
  static bhImage* CreateFromFile(const char* filePath, int reqComponents); // See definition of STBI_default -> stb_image.h
  static void Destroy(bhImage*& img);

  __forceinline int Width() const { return width; }
  __forceinline int Height() const { return height; }
  __forceinline int Depth() const { return depth; }
  __forceinline uint8_t* Pixels() { return pixels; }
  __forceinline const uint8_t* Pixels() const { return pixels; }
  __forceinline int Pitch() const { return width * numComponents; }

  int DetermineDimensions() const;
  bool IsValidForMipmap() const;
  uint32_t GetRequiredMipLevels() const;
  void Free();
  int PutPixel(int x, int y, uint32_t color);
  int SavePPM_Text(const char* path);

  __forceinline size_t PixelMemSiz() const
  {
    return sizeof(uint8_t) * numComponents;
  }

  __forceinline size_t MemSiz() const
  {
    return size_t(width) * size_t(height) * size_t(depth) * PixelMemSiz();
  }

  __forceinline bool ArePixelCoordsValid(int x, int y) const
  {
    return ((0 <= x) && (x < width)) && ((0 <= y) && (y < height));
  }

protected:
private:
  bhImage() = default;
  ~bhImage() = default;

  uint8_t* pixels{ nullptr }; // Muat match stbi_uc def --> unsigned char
  int width{ 0 }, height{ 0 }, depth{ 0 };
  int numComponents{ 0 };
};

#endif //BH_IMAGE_HPP
