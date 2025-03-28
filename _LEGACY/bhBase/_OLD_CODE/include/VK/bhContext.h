#ifndef BH_CONTEXT_H
#define BH_CONTEXT_H

#include "bhFramebuffer.h"
#include "Math/bhMatrix.hpp"

struct bhImage;

////////////////////////////////////////////////////////////////////////////////
class bhContext
{
public:
	VkRenderPass CreateRenderPass();

protected:
private:
	VkFence fence_draw = VK_NULL_HANDLE;
	VkClearValue clearValues_v[2]; // RenderPass
};

#endif //BH_CONTEXT_H
