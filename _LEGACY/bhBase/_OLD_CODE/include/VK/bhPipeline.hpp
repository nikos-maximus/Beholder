#ifndef BH_PIPELINE_HPP
#define BH_PIPELINE_HPP

#include "VK/bhRenderDevice.hpp"

class bhPipeline
{
public:
	VkBool32 Create(VkRenderPass renderPass, VkExtent2D size);
	void Destroy();
	//void BindGraphics(VkCommandBuffer cmdBuffer); // Maybe just retrieve the pipeline handle and issue the command?

protected:
private:
	VkPipelineLayout CreateLayout() const;

	const bhRenderDevice& device;
	VkPipelineLayout layout{ VK_NULL_HANDLE };
	VkPipeline pipeline{ VK_NULL_HANDLE };
};

#endif //BH_PIPELINE_HPP
