#include "bhImageTools.h"
#include <FreeImage.h>
#include <inttypes.h>
#include <assert.h>
#include <glm/glm.hpp>

namespace bhImageTools
{
	struct GreyScaleWeights
	{
		GreyScaleWeights()
			: r(0.0f), g(0.0f), b(0.0f)
		{}

		float r, g, b;
	};

	void InitLibraries()
	{
		FreeImage_Initialise();
	}

	void DestroyLibraries()
	{
		FreeImage_DeInitialise();
	}

	inline GreyScaleWeights CreateGreyscaleWeights(float r, float g, float b)
	{
		float norm = 1.0f / (r + g + b);
		GreyScaleWeights newGsw;
		newGsw.r = r * norm;
		newGsw.g = g * norm;
		newGsw.b = b * norm;
		return newGsw;
	}

	inline GreyScaleWeights CreateDefaultGreyscaleWeights()
	{
		GreyScaleWeights newGsw;
		newGsw.r = 0.2126f;
		newGsw.g = 0.7152f;
		newGsw.b = 1.0f - (newGsw.r + newGsw.g);
		return newGsw;
	}

	bool CreateHeightMap(FIBITMAP* bitmap, GreyScaleWeights& weights);

	struct ImageInfo
	{
		ImageInfo()
			: bitspp(0), bytespp(0)
			, w(0), h(0)
			, rmask(0), gmask(0), bmask(0)
			, fit(FIT_UNKNOWN)
		{}

		uint32_t bitspp, bytespp, w, h, rmask, gmask, bmask;
		FREE_IMAGE_TYPE fit;
	};

	ImageInfo CreateImageInfo(FIBITMAP* bitmap)
	{
		ImageInfo ii;
		ii.fit = FreeImage_GetImageType(bitmap);
		ii.bitspp = FreeImage_GetBPP(bitmap);
		ii.bytespp = ii.bitspp >> 3;
		ii.w = FreeImage_GetWidth(bitmap);
		ii.h = FreeImage_GetHeight(bitmap);
		ii.rmask = FreeImage_GetRedMask(bitmap);
		ii.gmask = FreeImage_GetGreenMask(bitmap);
		ii.bmask = FreeImage_GetBlueMask(bitmap);
		return ii;
	}

	bool LoadImage(char const* path)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(path);
		if (fif == FIF_UNKNOWN)
		{
			fif = FreeImage_GetFileType(path);
			if (fif == FIF_UNKNOWN)
			{
				return false;
			}
		}
		FIBITMAP* bitmap = FreeImage_Load(fif, path);
		if (bitmap == nullptr)
		{
			return false;
		}
		if (FreeImage_HasPixels(bitmap))
		{
			ImageInfo ii = CreateImageInfo(bitmap);
		}

		CreateHeightMap(bitmap, CreateDefaultGreyscaleWeights());
		//CreateHeightMap(bitmap, CreateGreyscaleWeights(2.0f, 0.0f, 0.0f)); //TEST

		FreeImage_Unload(bitmap);
		return true;
	}

	bool CreateHeightMap(FIBITMAP* bitmap, GreyScaleWeights& weights)
	{
		ImageInfo ii = CreateImageInfo(bitmap);
		FIBITMAP* heightMap = FreeImage_Allocate(ii.w, ii.h, ii.bitspp, ii.rmask, ii.gmask, ii.bmask);
		RGBQUAD inPixel, outPixel;
		const float inv = 1.0f / 255.0f;

		for (uint32_t y = 0; y < ii.h; ++y)
		{
			for (uint32_t x = 0; x < ii.w; ++x)
			{
				FreeImage_GetPixelColor(bitmap, x, y, &inPixel);
				float rf = (float(inPixel.rgbRed) * inv) * weights.r;
				float gf = (float(inPixel.rgbGreen) * inv) * weights.g;
				float bf = (float(inPixel.rgbBlue) * inv) * weights.b;
				outPixel.rgbRed = outPixel.rgbGreen = outPixel.rgbBlue = uint8_t((rf + gf + bf) * 255.0f);
				FreeImage_SetPixelColor(heightMap, x, y, &outPixel);
			}
		}

		FreeImage_Save(FIF_PNG, heightMap, "./test_h.png");
		FreeImage_Unload(heightMap);
		return true;
	}

	bool CreateNormalMap(FIBITMAP* heightMap)
	{
		ImageInfo ii = CreateImageInfo(heightMap);
		FIBITMAP* normalMap = FreeImage_Allocate(ii.w, ii.h, ii.bitspp, ii.rmask, ii.gmask, ii.bmask);

		RGBQUAD pixelN, pixelS, pixelE, pixelW, outPixel;

		for (uint32_t y = 0; y < ii.h; ++y)
		{
			for (uint32_t x = 0; x < ii.w; ++x)
			{
				FreeImage_GetPixelColor(heightMap, (x - 1 + ii.w) % ii.w, y, &pixelW);
				FreeImage_GetPixelColor(heightMap, (x + 1) % ii.w, y, &pixelE);
				FreeImage_GetPixelColor(heightMap, x, (y - 1 + ii.h) % ii.h, &pixelS);
				FreeImage_GetPixelColor(heightMap, x, (y + 1) % ii.h, &pixelN);

				glm::vec3 xvec(1.0f, 0.0f, (pixelE.rgbRed - pixelW.rgbRed) / 255.0f);
				glm::vec3 yvec(0.0f, 1.0f, (pixelN.rgbRed - pixelS.rgbRed) / 255.0f);
				glm::vec3 result = glm::normalize(glm::cross(xvec,yvec));

				FreeImage_SetPixelColor(normalMap, x, y, &outPixel);
			}
		}

		FreeImage_Save(FIF_PNG, normalMap, "./test_n.png");
		FreeImage_Unload(normalMap);
		return true;
	}
}
