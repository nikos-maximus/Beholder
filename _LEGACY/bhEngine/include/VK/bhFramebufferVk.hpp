#ifndef BH_FRAMEBUFFER_VK_HPP
#define BH_FRAMEBUFFER_VK_HPP

#include <vector>
#include "VK/bhTextureVk.hpp"

namespace bhVk
{
	struct RenderDevice;

	struct Framebuffer
	{
		VkFramebuffer framebuffer{ VK_NULL_HANDLE };
		VkImageView colorView{ VK_NULL_HANDLE };
		Texture depthStencilTexture;
		VkImageView depthStencilView{ VK_NULL_HANDLE };
		VkExtent2D extent{};
	};

	VkBool32 CreateFramebuffer(Framebuffer& fb, const RenderDevice& rd, VkExtent2D insize, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil);
	VkBool32 CreateFramebufferDepthStencilAttachment(Framebuffer& fb, const RenderDevice& rd);
	VkBool32 Instantiate(Framebuffer& fb, const RenderDevice& rd);
	void DestroyFramebuffer(Framebuffer& fb, const RenderDevice& rd);

	////////////////////////////////////////////////////////////////////////////////
	struct Swapchain
	{
		VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
		uint32_t swapChainIdx{ UINT32_MAX };
		std::vector<Framebuffer> frameBuffers_v;
	};
}

#endif //BH_FRAMEBUFFER_VK_HPP
