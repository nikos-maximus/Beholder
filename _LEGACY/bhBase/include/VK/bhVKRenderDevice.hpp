#ifndef BH_RENDER_DEVICE_HPP
#define BH_RENDER_DEVICE_HPP

#include "VK/bhVk.hpp"
#include "bhCamera.hpp"
#include "Mesh/bhMeshTypes.hpp"
#include "VK/bhVkWorldPipeline.hpp"

#if BH_USE_MESH_INDEX_TYPE_UNIT16
#define BH_MESH_INDEX_TYPE VK_INDEX_TYPE_UINT16
#else
#define BH_MESH_INDEX_TYPE VK_INDEX_TYPE_UINT32
#endif

struct SDL_Window;
class bhMesh;
class bhTexture;

namespace bhVk
{
	class Instance;

	class RenderDevice
	{
	public:
		RenderDevice(Instance& _instance);
		VkBool32 Init(VkSurfaceKHR wndSurf);
		void Destroy();
		void BeginFrame();
		void EndFrame();

		VkBool32 InitImGui(SDL_Window* wnd) const;
		void DestroyImGui() const;
		void BeginImGuiFrame() const;
		void EndImGuiFrame() const;

		void DestroyTexture(Texture& texture) const;

		VkBool32 CreateWorldPipeline(uint32_t w, uint32_t h);
		void DestroyWorldPipeline();
		void BindWorldPipeline();
		void SetWorldCameraView(const bhCamera::ViewData* vd);
		void UploadWorldMesh(bhMesh* mesh);
		void RenderWorldMesh(const bhMesh* mesh, const glm::mat4& transform);
		WorldPipeline::Material LoadWorldMaterial(const char* fileName) const;
		void DestroyWorldMaterial(const WorldPipeline::Material& mat) const;
		void BindWorldMaterial(const WorldPipeline::Material& mat);

	protected:
	private:
		RenderDevice() = delete;
		VkBool32 CreateDevice();
		VkBool32 ChoosePhysicalRenderDevice(const VkQueueFamilyProperties& reqQFP_Graphics, const std::vector<const char*>& reqExtNames_v);
		static uint32_t RankPhysicalDeviceGraphicsFeatures(const VkPhysicalDevice physicalDevice);
		VkBool32 CreateDescriptorPool();
		VkBool32 CreatePresentRenderPass();
		VkBool32 CreateSwapchain(uint32_t numImages);
		void DestroySwapchain();
		Texture CreateTexture(const VkImageCreateInfo& imageCI);
		//	VkBool32 CreateFramebuffer(Framebuffer& outFramebuffer, VkExtent2D size, uint32_t numColorAttachments, bool createDepthStencil);
		VkBool32 CreateFramebuffer(Framebuffer& outFramebuffer, VkExtent2D size, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil);
		VkBool32 CreateFramebufferColorAttachments(Framebuffer& outFramebuffer, uint32_t numColorAttachments);
		VkBool32 CreateFramebufferDepthStencilAttachment(Framebuffer& outFramebuffer);
		VkBool32 InstantiateFramebuffer(Framebuffer& outFramebuffer);
		void DestroyFramebuffer(Framebuffer& fb);
		VkBool32 CreateCommandBuffers(uint32_t numBuffers);

		VkBool32 CreateShaderStageFromFile(VkPipelineShaderStageCreateInfo& shaderStageCI, const char* fileName, const char* entryPoint = "main") const;

		VkPipelineLayout CreateWorldPipelineLayout();
		void DestroyWorldPipelineLayout();
		VkPipeline CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo) const;
		VkPipeline CreateComputePipeline(const VkComputePipelineCreateInfo& createInfo) const;
		void DestroyPipeline(VkPipeline pipeline) const;

		Buffer CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize) const;
		void DestroyBuffer(Buffer& buffer) const;
		VkBool32 CopyDataToBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data) const;

		Texture CreateMemoryBackedImage(const VkImageCreateInfo* imageCI) const;
		Texture CreateTexture(VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels) const;
		Buffer CreateBufferFromImage(const bhImage* img) const;
		Texture CreateTextureFromImage(const bhImage* img, int32_t offsetx, int32_t offsety) const;
		VkDescriptorImageInfo SetupTextureDescriptor(const bhTexture& tx) const;
		VkImageView CreateTextureImageView(const bhTexture& tx) const;

		WorldPipeline::Material CreateWorldMaterial(const WorldPipeline::MaterialCreateInfo& matCI) const;

		float GetAnisotropy() const;

		Instance& instance;
		VkSurfaceKHR wndSurface{ VK_NULL_HANDLE };
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkDevice device{ VK_NULL_HANDLE };
		VkAllocationCallbacks* allocator{ nullptr };
		uint32_t gfxQueueFamilyIdx{ UINT32_MAX };
		VkCommandPool commandPool{ VK_NULL_HANDLE };
		VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

		Swapchain swapChain;
		VkClearValue clearValues_v[2];
		std::vector<VkCommandBuffer> commandBuffers_v;

		VkRenderPass presentRenderPass{ VK_NULL_HANDLE };
		VkQueue renderQueue{ VK_NULL_HANDLE };

		VkFence drawFence{ VK_NULL_HANDLE };
		VkSemaphore sph_ImageAvailable{ VK_NULL_HANDLE };
		VkSemaphore sph_RenderFinished{ VK_NULL_HANDLE };

		size_t frameCount{ 0 };

		WorldPipeline worldPipeline;
	};
}

#endif //BH_RENDER_DEVICE_HPP
