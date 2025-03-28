#ifndef BH_VK_FRAMEBUFFER_HPP
#define BH_VK_FRAMEBUFFER_HPP

#include <vector>
#include "VK/bhVKTexture.hpp"

class bhVKRenderDevice;

class bhVKFramebuffer
{
public:
	VkBool32 Create(const bhVKRenderDevice* rd, VkExtent2D insize, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil);
	VkBool32 CreateColorAttachments(const bhVKRenderDevice* rd, uint32_t numColorAttachments);
	VkBool32 CreateDepthStencilAttachment(const bhVKRenderDevice* rd);
	VkBool32 Instantiate(const bhVKRenderDevice* rd);
	void Destroy(const bhVKRenderDevice* rd);

	inline const VkExtent2D& Size() const { return size; }
	inline const VkFramebuffer Framebuffer() const { return framebuffer; }

protected:
private:
	VkFramebuffer framebuffer{ VK_NULL_HANDLE };
	std::vector<bhVKTexture> colorTextures_v;
	std::vector<VkImageView> colorViews_v;
	bhVKTexture depthStencilTexture;
	VkImageView depthStencilView{ VK_NULL_HANDLE };
	VkExtent2D size = {};
	bool deleteColorBuffers{ true };
};

#endif //BH_VK_FRAMEBUFFER_HPP
