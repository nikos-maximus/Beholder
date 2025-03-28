#pragma once
#include <volk.h>
#include <vector>

class bhRenderPass
{
public:
protected:
private:
	VkExtent2D framebufferSize = {};
	VkRenderPass renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> framebuffers_v;
};
