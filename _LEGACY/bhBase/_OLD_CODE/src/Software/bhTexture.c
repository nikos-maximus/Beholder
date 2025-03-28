#include <assert.h>
#define STB_IMAGE_IMPLEMENTATION
#include "bhImage.h"
#include "Software/bhPrimitives.h"

uint32_t bhSampleTexture(bhImage const* img, float u, float v)
{
    assert(img->numComponents== 3 || img->numComponents == 4);
    int iu = (int)(u * img->width) % img->width;
    int iv = (int)(v * img->height) % img->height;
    int pxOffs = (iv * img->width + iu) * img->numComponents;
    stbi_uc* pixel = &(img->pixels[pxOffs]);

    return bhColor3ub(pixel[0], pixel[1], pixel[2]);
}
