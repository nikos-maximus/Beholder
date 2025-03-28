#ifndef BH_RENDERER_VK_HPP
#define BH_RENDERER_VK_HPP

#include <array>
#include "VK/bhStructsVK.hpp"
#include "VK/bhPipelineVK.hpp"
#include "Private/bhGPUDevice.hpp"
#include "Private/bhResourceManager.hpp"

////////////////////////////////////////////////////////////////////////////////
class bhDeviceVK : public bhGPUDevice
{
public:
	void BeginFrame() override final;
	void EndFrame() override final;

	bhResourceID CreateFramebuffer(bhSize2Di size, uint32_t numColorAttachments, bool createDepthStencil) override final;
	void DestroyFramebuffer(bhResourceID fbId) override final;
	void UseFramebuffer(bhResourceID fbId) override final;
	void UseWindowFramebuffer() override final {}
	void* GetFramebufferColorAttachment(bhResourceID fbId, size_t attachmentIdx) override final;
	bhSize2Di GetFramebufferSize(bhResourceID fbId) override final;

	bhResourceID CreateTextureFromImage(const bhImage* image, const char* name, int32_t offsetx, int32_t offsety) override final;

	bhBufferVK CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize);
	bhBufferVK CreateBufferFromImage(const bhImage* img);
	void DestroyBuffer(bhBufferVK* buffer);
	VkBool32 CopyDataToBuffer(const bhBufferVK* buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data);

	bool CreateShaderFromFile(const char* fileName, VkShaderModule* newShaderModule, VkShaderStageFlagBits* newShaderType);
	VkSampler CreateSampler(const VkSamplerCreateInfo* createInfo) const;
	void DestroySampler(VkSampler sampler);

	bhResourceID CreatePipeline_World() override;
	bool IsPipelineValid_World(bhResourceID pipelineID) override;
	void UsePipeline_World(bhResourceID pipelineID) override;
	void DestroyPipeline_World(bhResourceID pipelineID) override;
	
	bhResourceID CreateMaterial_World(bhResourceID pipelineID, const bhWorldMaterialCreateInfo* materialCI) override;
	void UseMaterial_World(bhResourceID materialID) override;
	void DestroyMaterial_World(bhResourceID materialID) override;

	void UploadMesh_World(bhResourceID pipelineID, bhResourceID meshID) override;
	void RenderMesh_World(bhResourceID pipelineID, bhResourceID meshID, const glm::mat4* transform) override;
	void SetCameraView_World(bhResourceID pipelineID, const bhCamera::ViewData* viewProjection) override;

#ifdef BH_USE_IMGUI
	bool InitImGui() override;
	void DestroyImGui() override;
	void BeginImGuiFrame() override;
	void EndImGuiFrame() override;
#endif //BH_USE_IMGUI

protected:
	bool Init() override final;
	void Destroy() override final;

private:
	static VkImageType MapImageTypeToVulkan(int dim);
	static void GenerateMipmaps(VkCommandBuffer commandBuffer, const bhImage* img, uint32_t numMipLevels, bhTextureVK* texture);
	static VkFormat PickImageFormat(const bhImage* img);
	static uint32_t RankPhysicalDeviceGraphicsFeatures(const VkPhysicalDevice& physicalDevice);
	static VkExtent2D GetMaxFramebufferSize(VkPhysicalDevice physicalDevice);

	uint32_t CreateSwapchain(bhSwapchainVK* outSwapchain, uint32_t numImages);
	void DestroySwapchain(bhSwapchainVK* bhSwapchain);
	static void ClearSwapchain(bhSwapchainVK* bhSwapchain);
	VkBool32 CreateDescriptorPool();
	VkRenderPass CreateRenderPass();
	bool ChoosePhysicalRenderDevice(const VkQueueFamilyProperties* reqQFP_Graphics, const std::vector<const char*>* reqExtNames_v);
	bool CreateRenderDevice();
	float GetAnisotropy() const;
	bhTextureVK* CreateMemoryBackedImage(const VkImageCreateInfo* imageCI);
	bhTextureVK* CreateTexture(VkImageType imageType, VkFormat fmt, const VkExtent3D& extent, uint32_t mipLevels);
	bhTextureVK* CreateTextureFromImage_Local(const bhImage* img, int32_t offsetx, int32_t offsety);
	void DestroyTexture(bhTextureVK* texture);

	VkSampler bhWorldMaterial_CreateSampler() const;
	VkImageView bhWorldMaterial_CreateTextureView(const bhImage* img, const bhTextureVK* texture) const;
	VkDescriptorImageInfo bhWorldMaterial_SetupTextureInput(const std::string* textureName);
	bool bhWorldPipeline_CreateLayout(bhPipelineVK_World* outPipeline);
	void bhWorldPipeline_CreateDataBuffers(bhPipelineVK_World* pipeline);
	void bhWorldPipeline_DestroyDataBuffers(bhPipelineVK_World* pipeline);
	void UseMaterial_World(const bhMaterialVK_World* mat);

	void CreateFramebuffer_Common(bhFramebufferVK* outFramebuffer, bool createDepthStencil);
	bhResourceID CreateFramebuffer(VkExtent2D _size, VkImage image, VkSurfaceFormatKHR imgFormat, bool createDepthStencil);

	////////////////////////////////////////
	// DEVICE
	bhInstanceVK instanceVK;

	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	VkDevice device{ VK_NULL_HANDLE };
	VkAllocationCallbacks* deviceAllocator{ nullptr };
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
	uint32_t gfxQueueFamilyIdx { UINT32_MAX };

	std::array<VkClearValue, 2> clearValues_v;
	VkQueue renderQueue{ VK_NULL_HANDLE };
	bhSwapchainVK swapChain;
	std::vector<VkCommandBuffer> commandBuffers_v;
	VkFence drawFence{ VK_NULL_HANDLE };

	VkSurfaceKHR windowSurface{ VK_NULL_HANDLE };

	VkRenderPass renderPass{ VK_NULL_HANDLE };

	VkSemaphore sph_ImageAvailable{ VK_NULL_HANDLE };
	VkSemaphore sph_RenderFinished{ VK_NULL_HANDLE };

	bhResourceManager<bhTextureVK> textureManager;
};

#endif //BH_RENDERER_VK_HPP
