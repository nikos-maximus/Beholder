#define STB_IMAGE_IMPLEMENTATION

#include "Texture/bhImage.h"
#include "Math/bhMathUtil.h"

////////////////////////////////////////////////////////////////////////////////
__forceinline int bhImage_ArePixelCoordsValid(const bhImage* img, int x, int y)
{
    return ((0 <= x) && (x < img->width)) && ((0 <= y) && (y < img->height)) ? 1 : 0;
}

void bhImage_Init()
{
    stbi_set_flip_vertically_on_load(1);
    // SEE ALSO:
    //stbi_set_flip_vertically_on_load_thread
}

int bhImage_DetermineDimensions(const bhImage* img)
{
    int dim = 0;
    if (img->width > 0)
    {
        ++dim;
        if (img->height > 1)
        {
            ++dim;
            if (img->depth > 1)
            {
                ++dim;
            }
        }
    }
    assert((0 < dim) && (dim < 4));
    return dim;
}

void bhImage_Delete(bhImage* img)
{
    stbi_image_free(img->pixels);
    img->width = img->height = img->depth = 0;
    img->numComponents = 0;
}

bhImage* bhImage_CreateEmpty(int w, int h, int numComponents)
{
    bhImage* img = (bhImage*)calloc(1, sizeof(bhImage));
    if (img == NULL)
    {
        return NULL;
    }
    img->width = w;
    img->height = h;
    img->depth = 1;
    img->numComponents = numComponents;
    img->pixels = (stbi_uc*)calloc(w * h, img->numComponents);
    if (img->pixels == NULL)
    {
        free(img);
        img = NULL;
    }
    return img;
}

bhImage* bhImage_CreateFromFile(const char* filePath, int reqComponents)
{
    assert((0 <= reqComponents) && (reqComponents <= STBI_rgb_alpha));

    bhImage* img = (bhImage*)calloc(1, sizeof(bhImage));
    img->pixels = stbi_load(filePath, &(img->width), &(img->height), &(img->numComponents), reqComponents);
    if (img->pixels == NULL)
    {
        free(img);
        return NULL;
    }
    img->depth = 1;
    if ((reqComponents > 0) && (reqComponents != img->numComponents))
    {
        img->numComponents = reqComponents;
        img->retCode = 1; // A chance to warn about runtime conversion
    }
    return img;
}

int bhImage_IsValidForMipmap(const bhImage* img)
{
    // Assumptions:
    // 1D images must be of power-of-2 width
    // 2D images must be square, power-of-2 width AND height
    int dim = bhImage_DetermineDimensions(img);
    switch (dim)
    {
        case 1:
        {
            return bhMath_IsPowerOf2(img->width);
        }
        case 2:
        {
            return bhMath_IsPowerOf2(img->width) && bhMath_IsPowerOf2(img->height) && (img->width == img->height);
        }
        case 3:
        default:
        {
            break;
        }
    }
    return 0;
}

uint32_t bhImage_GetRequiredMipLevels(const bhImage* img)
{
    if (!bhImage_IsValidForMipmap(img))
    {
        return 1;
    }
    uint32_t mipLevels = 1;
    {
        switch (bhImage_DetermineDimensions(img))
        {
            case 1:
            case 2:
            {
                int mipWidth = img->width;
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

int bhImage_SavePPM_Text(const bhImage* img, const char* path)
{
    assert(img->numComponents >= 3);

    FILE* file = NULL;
    int result = fopen_s(&file, path, "w");
    if (result == 0)
    {
        fprintf(file, "P3\n%d %d\n255\n", img->width, img->height);
        stbi_uc* pixel = img->pixels;
        for (int row = 0; row < img->height; ++row)
        {
            for (int col = 0; col < img->width; ++col)
            {
                fprintf(file, "%d %d %d\n", *pixel, *(pixel + 1), *(pixel + 2));
                pixel += img->numComponents;
            }
        }
        fclose(file);
    }
    return result;
}

int bhImage_PutPixel(const bhImage* img, int x, int y, uint32_t color)
{
    assert(img->numComponents == 4);

    int result = bhImage_ArePixelCoordsValid(img, x, y);
    if (result)
    {
        stbi_uc* pixel = img->pixels + ((y * img->width) + x) * img->numComponents;
        *(uint32_t*)pixel = color;
        //for (int c = 0; c < img->numComponents; ++c)
        //{
        //    *pixel = color & 0xFF;
        //    color = color >> 1;
        //}
    }
    return result;
}
