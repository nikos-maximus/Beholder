#define STB_IMAGE_IMPLEMENTATION
#include "bhImage.h"
#include <glm/geometric.hpp>
#include "Software/bhRay.hpp"

bhImage* CreateGradientImage(int w, int h)
{
	bhImage* img = bhImage_CreateEmpty(w, h, 3);
	if (img == NULL)
	{
		return NULL;
	}

	float gradientXStep = 1.f / (float)w;
	float gradientYStep = 1.f / (float)h;

	float gradientXVal = gradientXStep;
	float gradientYVal = gradientYStep;
	stbi_uc* pixel = img->pixels;
	for (int row = 0; row < img->height; ++row)
	{
		gradientXVal = gradientXStep;
		for (int col = 0; col < img->width; ++col)
		{
			*(pixel + 0) = (stbi_uc)(gradientXVal * 255.f);
			*(pixel + 1) = (stbi_uc)(gradientYVal * 255.f);
			*(pixel + 2) = 0x00;
			pixel += img->numComponents;
			gradientXVal += gradientXStep;
		}
		gradientYVal += gradientYStep;
	}

	return img;
}

glm::vec3 Color(const bhRay& ray)
{
	glm::vec3 unit_direction = glm::normalize(ray.GetDirection());
	return unit_direction;
}

int main(int argc, char* argv[])
{
	const int WINDOW_W = 256;
	const int WINDOW_H = 128;

	//bhImage* img = CreateGradientImage(256, 128);
	bhImage* img = bhImage_CreateEmpty(WINDOW_W, WINDOW_H, 4);
	assert(img != NULL);

	const glm::vec3 origin(0.f, 0.f, 0.f);
	const glm::vec3 lowerLeft(-2.f, -1.f, -1.f);
	const float cameraWidth = 4.f;
	const float cameraHeight = 2.f;

	float horzStep = cameraWidth / float(WINDOW_W);
	float vertStep = cameraHeight / float(WINDOW_H);

	glm::vec3 newDirection = lowerLeft;
	for (int row = 0; row < img->height; ++row)
	{
		newDirection.x = lowerLeft.x;
		for (int col = 0; col < img->width; ++col)
		{
			bhRay ray(origin, newDirection);
			glm::vec3 color = Color(ray);
			uint32_t iColor = int(color.r * 255) << 16 | int(color.g * 255) << 8 | int(color.b * 255);
			bhImage_PutPixel(img, col, row, iColor);
			newDirection.x += horzStep;
		}
		newDirection.y += vertStep;
	}

	bhImage_SavePPM_Text(img, "./Image.ppm");
	bhImage_Delete(img);
	return 0;
}
