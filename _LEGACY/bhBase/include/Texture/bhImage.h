#ifndef BH_IMAGE_H
#define BH_IMAGE_H

#include <stdint.h>
#include <stb_image.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
typedef struct bhImage
{
    stbi_uc* pixels;
    int width, height, depth;
    int numComponents;
    int retCode;
}
bhImage;

void bhImage_Init();

int bhImage_DetermineDimensions(const bhImage* img);

__forceinline size_t bhImage_GetPixelMemSiz(const bhImage* img)
{
    return sizeof(stbi_uc) * img->numComponents;
}

__forceinline size_t bhImage_GetMemSiz(const bhImage* img)
{
    return img->width * img->height * img->depth * bhImage_GetPixelMemSiz(img);
}

bhImage* bhImage_CreateEmpty(int w, int h, int numComponents);
bhImage* bhImage_CreateFromFile(const char* filePath, int reqComponents); // See definition of STBI_default -> stb_image.h
int bhImage_IsValidForMipmap(const bhImage* img);
uint32_t bhImage_GetRequiredMipLevels(const bhImage* img);
void bhImage_Delete(bhImage* img);
int bhImage_PutPixel(const bhImage* img, int x, int y, uint32_t color);
int bhImage_SavePPM_Text(const bhImage* img, const char* path);

#ifdef __cplusplus
}
#endif //extern "C"

#endif //BH_IMAGE_H
