#include <SDL3/SDL_log.h>
#include <stb_image.h>
#include "bhImage.hpp"
#include "bhMathUtil.hpp"

////////////////////////////////////////////////////////////////////////////////
void bhImage::InitEnv()
{
  stbi_set_flip_vertically_on_load(1);
  // SEE ALSO:
  //stbi_set_flip_vertically_on_load_thread
}

int bhImage::DetermineDimensions() const
{
  int dim = 0;
  if (width > 0)
  {
    ++dim;
    if (height > 1)
    {
      ++dim;
      if (depth > 1)
      {
        ++dim;
      }
    }
  }
  assert((0 < dim) && (dim < 4));
  return dim;
}

void bhImage::Free()
{
  stbi_image_free(pixels);
  pixels = nullptr;
  width = height = depth = 0;
  numComponents = 0;
}

bhImage* bhImage::CreateEmpty(int w, int h, int numComponents)
{
  bhImage* img = (bhImage*)calloc(1, sizeof(bhImage));
  if (img == nullptr)
  {
    return nullptr;
  }
  img->width = w;
  img->height = h;
  img->depth = 1;
  img->numComponents = numComponents;
  img->pixels = (stbi_uc*)calloc(w * h, img->numComponents);
  if (img->pixels == nullptr)
  {
    free(img);
    img = nullptr;
  }
  return img;
}

bhImage* bhImage::CreateFromFile(const char* filePath, int reqComponents)
{
  assert((0 <= reqComponents) && (reqComponents <= STBI_rgb_alpha));

  bhImage* img = (bhImage*)calloc(1, sizeof(bhImage));
  if (img)
  {
    img->pixels = stbi_load(filePath, &(img->width), &(img->height), &(img->numComponents), reqComponents);
    if (img->pixels == nullptr)
    {
      free(img);
      SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_ERROR, "Could not load image %s", filePath);
      return nullptr;
    }
    img->depth = 1;
    if ((reqComponents > 0) && (reqComponents != img->numComponents))
    {
      img->numComponents = reqComponents;
      SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_WARN, "Loading image %s: Runtime conversion", filePath);
    }
  }
  return img;
}

bool bhImage::IsValidForMipmap() const
{
  // Assumptions:
  // 1D images must be of power-of-2 width
  // 2D images must be square, power-of-2 width AND height
  int dim = DetermineDimensions();
  switch (dim)
  {
    case 1:
    {
      return bhMath_IsPowerOf2(width);
    }
    case 2:
    {
      return bhMath_IsPowerOf2(width) && bhMath_IsPowerOf2(height) && (width == height);
    }
    case 3:
    default:
    {
      break;
    }
  }
  return 0;
}

uint32_t bhImage::GetRequiredMipLevels() const
{
  if (!IsValidForMipmap())
  {
    return 1;
  }
  uint32_t mipLevels = 1;
  {
    switch (DetermineDimensions())
    {
      case 1:
      case 2:
      {
        int mipWidth = width;
        while (mipWidth > 1)
        {
          mipWidth /= 2;
          ++mipLevels;
        }
        break;
      }
      case 3:
      default:
      {
        break;
      }
    }
  }
  return mipLevels;
}

int bhImage::SavePPM_Text(const char* path)
{
  assert(numComponents >= 3);

  FILE* file = nullptr;
  int result = fopen_s(&file, path, "w");
  if (result == 0)
  {
    fprintf(file, "P3\n%d %d\n255\n", width, height);
    stbi_uc* pixel = pixels;
    for (int row = 0; row < height; ++row)
    {
      for (int col = 0; col < width; ++col)
      {
        fprintf(file, "%d %d %d\n", *pixel, *(pixel + 1), *(pixel + 2));
        pixel += numComponents;
      }
    }
    fclose(file);
  }
  return result;
}

int bhImage::PutPixel(int x, int y, uint32_t color)
{
  assert(numComponents == 4);

  int result = ArePixelCoordsValid(x, y);
  if (result)
  {
    stbi_uc* pixel = pixels + ((y * width) + x) * numComponents;
    *(uint32_t*)pixel = color;
    //for (int c = 0; c < numComponents; ++c)
    //{
    //    *pixel = color & 0xFF;
    //    color = color >> 1;
    //}
  }
  return result;
}
