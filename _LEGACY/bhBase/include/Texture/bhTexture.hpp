#ifndef BH_TEXTURE_HPP
#define BH_TEXTURE_HPP

#include <memory>

#if BH_GPU_API_VULKAN
	#include "VK/bhVK.hpp"
	typedef bhVk::Texture DeviceTexture;
#elif BH_GPU_API_OPENGL
	#include "GL/bhGl.hpp"
	typedef bhGl::Texture DeviceTexture;
#endif

#include "bhImage.h"

class bhTexture
{
public:
	bool Load(const char* fileName);
	const bhImage* GetImage() const { return image.get(); }
	void SetTexture(const DeviceTexture& _dt) { texture = _dt; }
	const DeviceTexture& GetTexture() const { return texture; }
	DeviceTexture& GetTexture() { return texture; }

protected:
private:
	std::unique_ptr<bhImage> image;
	DeviceTexture texture;
};

#endif //BH_TEXTURE_HPP
