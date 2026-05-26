#include <vector>
//#include <iostream> //DEBUG
//#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_vulkan.h>
//
//#include "bhDefines.hpp"
#include "bhVk.hpp"
#include "bhVkPipeline.hpp"
#include "bhMesh.hpp"
//#include "bhUtil.hpp"
#include "bhPlatform.hpp"
//#include "bhLog.hpp"
//
#ifdef SDL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif
//
//#include <imgui.h>
//#include <imgui_impl_sdl3.h>
//#include <imgui_impl_vulkan.h>
//
//#include <ktx.h>
//#include <ktxvulkan.h>

namespace bhVk
{
	extern VkFormat g_presentImageFormat, g_depthStencilFormat;
	static constexpr uint32_t NUM_TEXTURE_SAMPLERS{ 2 };

	////////////////////////////////////////////////////////////////////////////////
	PipelineLayout CreatePipelineLayout(RenderDevice* rd)
	{
		VkDescriptorBindingFlags descBindFlags{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };

		VkDescriptorSetLayoutBinding descLayoutBindingTex{};
		{
			descLayoutBindingTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descLayoutBindingTex.descriptorCount = NUM_TEXTURE_SAMPLERS;
			descLayoutBindingTex.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		VkDescriptorSetLayoutCreateInfo descLayoutTexCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		{
			descLayoutTexCI.pNext = &descBindFlags;
			descLayoutTexCI.bindingCount = 1;
			descLayoutTexCI.pBindings = &descLayoutBindingTex;
		}
		VkDescriptorSetLayout dsl = rd->CreateDescriptorSetLayout(descLayoutTexCI);
		if (!dsl)
		{
			return {};
		}

		VkPushConstantRange pushConstRange{};
		{
			pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			pushConstRange.size = sizeof(VkDeviceAddress);
		}

		VkPipelineLayoutCreateInfo layoutCI{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		{
			layoutCI.setLayoutCount = 1;
			layoutCI.pSetLayouts = &dsl;
			layoutCI.pushConstantRangeCount = 1;
			layoutCI.pPushConstantRanges = &pushConstRange;
		}
		VkPipelineLayout pl = rd->CreatePipelineLayout(layoutCI);
		if (!pl)
		{
			return {};
		}
		return { dsl, pl };
	}

	void DestroyPipelineLayout(RenderDevice* rd, PipelineLayout& pl)
	{
		rd->DestroyPipelineLayout(pl.pl);
		rd->DestroyDescriptorSetLayout(pl.dsl);
	}

	////////////////////////////////////////////////////////////////////////////////
	bool GraphicsPipeline::Create(RenderDevice* rd)
	{
		////////////////////////////////////////////////////////////////////////////////
		//Rendering
		VkPipelineRenderingCreateInfo renderingCI{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		{
			renderingCI.colorAttachmentCount = 1;
			renderingCI.pColorAttachmentFormats = &g_presentImageFormat;
			renderingCI.depthAttachmentFormat = g_depthStencilFormat;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Shader state
		VkShaderModule shader01 = rd->CreateShaderModule(bhPlatform::CreateResourcePath(bhPlatform::ResourceType::RT_SHADER, "Shader01.out"));
		if (!shader01) return false;

		VkPipelineShaderStageCreateInfo shaderStageCI[2]{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		{
			shaderStageCI[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderStageCI[0].module = shader01;
			shaderStageCI[0].pName = "main";

			shaderStageCI[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStageCI[1].module = shader01;
			shaderStageCI[1].pName = "main";
		}

		////////////////////////////////////////////////////////////////////////////////
		//Vertex input state
		VkVertexInputBindingDescription vertexBindingDesc{};
		{
			vertexBindingDesc.binding = 0;
			vertexBindingDesc.stride = sizeof(bhMesh::Vertex);
			vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

		std::vector<VkVertexInputAttributeDescription> vertexAttribs(4);
		{
			//Position
			vertexAttribs[0].location = 0;
			vertexAttribs[0].binding = 0;
			vertexAttribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			//vertexAttribs[0].offset = ; //TODO: Check runtime value

			//Normal
			vertexAttribs[1].location = 1;
			vertexAttribs[1].binding = 0;
			vertexAttribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexAttribs[1].offset = offsetof(bhMesh::Vertex, bhMesh::Vertex::normal);

			//Tangent
			vertexAttribs[2].location = 2;
			vertexAttribs[2].binding = 0;
			vertexAttribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexAttribs[2].offset = offsetof(bhMesh::Vertex, bhMesh::Vertex::tangent);

			//UV0
			vertexAttribs[3].location = 3;
			vertexAttribs[3].binding = 0;
			vertexAttribs[3].format = VK_FORMAT_R32G32_SFLOAT;
			vertexAttribs[3].offset = offsetof(bhMesh::Vertex, bhMesh::Vertex::uv0);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputStateCI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		{
			vertexInputStateCI.vertexBindingDescriptionCount = 1;
			vertexInputStateCI.pVertexBindingDescriptions = &vertexBindingDesc;
			vertexInputStateCI.vertexAttributeDescriptionCount = uint32_t(vertexAttribs.size());
			vertexInputStateCI.pVertexAttributeDescriptions = vertexAttribs.data();
		}

		////////////////////////////////////////////////////////////////////////////////
		//Input assembly state
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		{
			inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Viewport state
		VkPipelineViewportStateCreateInfo viewportStateCI{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		{
			viewportStateCI.viewportCount = 1;
			viewportStateCI.scissorCount = 1;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationStateCI{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		{
			rasterizationStateCI.lineWidth = 1.0f;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Multisample state
		VkPipelineMultisampleStateCreateInfo multisampleStateCI{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		{
			multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		}

		////////////////////////////////////////////////////////////////////////////////
		//DepthStencil state
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		{
			depthStencilStateCI.depthTestEnable = VK_TRUE;
			depthStencilStateCI.depthWriteEnable = VK_TRUE;
			depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Colorblend state
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		{
			colorBlendAttachment.colorWriteMask = 0xF;
		}

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		{
			colorBlendStateCI.attachmentCount = 1;
			colorBlendStateCI.pAttachments = &colorBlendAttachment;
		}

		////////////////////////////////////////////////////////////////////////////////
		//Dynamic state
		std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCI{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		{
			dynamicStateCI.dynamicStateCount = uint32_t(dynamicStates.size());
			dynamicStateCI.pDynamicStates = dynamicStates.data();
		}

		////////////////////////////////////////////////////////////////////////////////
		//Layout

		////////////////////////////////////////////////////////////////////////////////
		VkGraphicsPipelineCreateInfo graphicsPipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		{
			graphicsPipelineCI.pNext = &renderingCI;
			graphicsPipelineCI.stageCount = 2;
			graphicsPipelineCI.pStages = shaderStageCI;
			graphicsPipelineCI.pVertexInputState = &vertexInputStateCI;
			graphicsPipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
			graphicsPipelineCI.pViewportState = &viewportStateCI;
			graphicsPipelineCI.pRasterizationState = &rasterizationStateCI;
			graphicsPipelineCI.pMultisampleState = &multisampleStateCI;
			graphicsPipelineCI.pDepthStencilState = &depthStencilStateCI;
			graphicsPipelineCI.pColorBlendState = &colorBlendStateCI;
			graphicsPipelineCI.pDynamicState = &dynamicStateCI;
			graphicsPipelineCI.layout = pl.pl;
			//gpCI.renderPass = ;
			//gpCI.subpass = ;
			//gpCI.basePipelineHandle = ;
			//gpCI.basePipelineIndex = ;
		}
		return rd->CreatePipeline(graphicsPipelineCI);
	}

	void GraphicsPipeline::Destroy(RenderDevice* rd)
	{
		rd->DestroyPipeline(pipeline);
		pipeline = VK_NULL_HANDLE;
	}

	void GraphicsPipeline::Bind(const VkCommandBuffer& cb) const
	{
		vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void GraphicsPipeline::BindDescriptorSet(const VkCommandBuffer& cb, const VkDescriptorSet& ds) const
	{
		vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pl.pl, 0, 1, &ds, 0, nullptr);
	}
}
