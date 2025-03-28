#include "Texture/bhTexture.hpp"
#include "bhLog.h"
#include "Platform/bhPlatform.hpp"

bool bhTexture::Load(const char* fileName)
{
	const char* path = bhPlatform::CreateResourcePath(bhPlatform::RT_TEXTURE, fileName);
	image.reset(bhImage_CreateFromFile(path, STBI_rgb_alpha)); // TODO: Revisit this - do we always need 4-component?
	bhPlatform::FreePath(path);
	if (!image)
	{
		bhLog_Message(bhLogPriority::LP_ERROR, "Could not load image %s", fileName);
		return false;
	}
	if (image->retCode == 1)
	{
		bhLog_Message(bhLogPriority::LP_WARN, "Loading image %s: Runtime conversion", fileName);
	}
	return true;
}
