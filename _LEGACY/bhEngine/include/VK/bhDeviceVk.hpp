#ifndef BH_VK_DEVICE_HPP
#define BH_VK_DEVICE_HPP

#include <glm/mat4x4.hpp>
#include "VK/bhTypesVk.hpp"
#include "VK/bhFramebufferVk.hpp"

struct SDL_Window;

class bhMesh;

namespace bhVk
{
	struct Instance;
	struct Mesh;

	////////////////////////////////////////////////////////////////////////////////
	struct Device
	{
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkDevice device{ VK_NULL_HANDLE };
		VkAllocationCallbacks* alloc{ nullptr };
		VmaAllocator allocator{ VK_NULL_HANDLE };
		VkCommandPool commandPool{ VK_NULL_HANDLE };
		VkQueue queue{ VK_NULL_HANDLE };
	};

	Buffer CreateBuffer(const Device& dev, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize);
	void DestroyBuffer(const Device& dev, Buffer& buffer);
	VkBool32 CopyDataToBuffer(const Device& dev, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data);

	VkCommandBuffer BeginSingleTimeCommandBuffer(const Device& dev);
	void EndSingleTimeCommandBuffer(const Device& dev, VkCommandBuffer commandBuffer, VkQueue queue);
	bool CreateTexture(const Device& dev, Texture& newTex, const VkImageCreateInfo& imageCI);
	bool CreateTexture(const Device& dev, Texture& newTex, VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels);
	bool CreateTextureFromImage(const Device& dev, Texture& newTex, const bhImage& img, int32_t offsetx, int32_t offsety);
	void DestroyTexture(const Device& dev, Texture& tex, bool destroyColorImage = true);

	////////////////////////////////////////////////////////////////////////////////
	struct RenderDevice : public Device
	{
		uint32_t queueFamilyIdx{ UINT32_MAX };
		VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

		Swapchain swapChain;
		VkClearValue clearValues_v[2];
		std::vector<VkCommandBuffer> commandBuffers_v;

		VkRenderPass presentRenderPass{ VK_NULL_HANDLE };

		VkFence drawFence{ VK_NULL_HANDLE };
		VkSemaphore sph_ImageAvailable{ VK_NULL_HANDLE };
		VkSemaphore sph_RenderFinished{ VK_NULL_HANDLE };

		size_t frameCount{ 0 };

		Buffer vertexBuffer;
		Buffer indexBuffer;
	};

	VkBool32 CreateRenderDevice(RenderDevice& dev, const Instance& inst, const VkSurfaceKHR wndSurf);
	RenderDevice& GetRenderDevice();
	void DestroyRenderDevice(RenderDevice& dev, const Instance& inst);

	void BeginFrame(RenderDevice& dev);
	void EndFrame(RenderDevice& dev);

	VkShaderModule CreateShaderFromFile(const RenderDevice& dev, const char* fileName);
	VkDescriptorImageInfo SetupTextureDescriptor(const RenderDevice& dev, const Texture& tx);

	bool CreateImGui(const RenderDevice& dev, const Instance& inst, SDL_Window* wnd);
	void DestroyImGui();
	void BeginImGuiFrame(RenderDevice& dev);
	void EndImGuiFrame(RenderDevice& dev);

	VkBool32 CreateDeviceMesh(Mesh& devMesh, const RenderDevice& dev, const bhMesh& mesh);
	void DestroyDeviceMesh(const RenderDevice& dev, Mesh& devMesh);
	void BindDeviceMesh(const RenderDevice& dev, const bhVk::Mesh& mesh);
	void DrawDeviceMesh(const RenderDevice& dev, VkPipelineLayout plLayout, const bhVk::Mesh& mesh, const glm::mat4& transform);
}

#endif //BH_VK_DEVICE_HPP
