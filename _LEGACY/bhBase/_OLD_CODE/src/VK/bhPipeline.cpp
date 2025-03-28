#include "VK/bhPipeline.hpp"
#include "Mesh/bhMesh.hpp" //bhWorldVertex
#include <glm/mat4x4.hpp>

VkPipelineLayout bhPipeline::CreateLayout() const
{
	std::vector<VkDescriptorSetLayout> descSetLayouts_v;
	VkDescriptorSetLayout tmpLayout = VK_NULL_HANDLE;

	// View / Projection layout
	{
		std::vector<VkDescriptorSetLayoutBinding> viewProjectionBindings_v
		{
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
		};
		VkDescriptorSetLayoutCreateInfo viewProjectionCI =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
			static_cast<uint32_t>(viewProjectionBindings_v.size()),
			viewProjectionBindings_v.data()
		};
		tmpLayout = device.CreateDescriptorSetLayout(viewProjectionCI);
		assert(tmpLayout != VK_NULL_HANDLE);
		descSetLayouts_v.push_back(tmpLayout);
	}

#if PIPELINE_MODEL // Here we are using a uniform buffer for object transformations instead of push constants
	// Model layout
	{
		std::vector<VkDescriptorSetLayoutBinding> modelBindings_v
		{
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
		};
		VkDescriptorSetLayoutCreateInfo modelCI =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
			static_cast<uint32_t>(modelBindings_v.size()),
			modelBindings_v.data()
		};
		tmpLayout = device.CreateDescriptorSetLayout(modelCI);
		assert(tmpLayout != VK_NULL_HANDLE);
		descSetLayouts_v.push_back(tmpLayout);
	}
#endif

	// Textures layout
	{
		std::vector<VkDescriptorSetLayoutBinding> textureBindings_v
		{
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },	// ALBEDO (RGBA)
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }	// NORMAL (RGB) + SPECULAR (A)
		};
		VkDescriptorSetLayoutCreateInfo texturesCI =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
			(uint32_t)textureBindings_v.size(),
			textureBindings_v.data()
		};
		tmpLayout = device.CreateDescriptorSetLayout(texturesCI);
		assert(tmpLayout != VK_NULL_HANDLE);
		descSetLayouts_v.push_back(tmpLayout);	}

	////////////////////////////////////////////////////////////////////////////////
	// Push Constants
	std::vector<VkPushConstantRange> pushConstantRanges_v =
	{
		{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) }
	};

	////////////////////////////////////////////////////////////////////////////////
	VkPipelineLayoutCreateInfo pipelineLayoutCI =
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0,
		static_cast<uint32_t>(descSetLayouts_v.size()),
		descSetLayouts_v.data(),
		static_cast<uint32_t>(pushConstantRanges_v.size()),
		pushConstantRanges_v.data()
	};

	return device.CreatePipelineLayout(pipelineLayoutCI);
}

VkBool32 bhPipeline::Create(VkRenderPass renderPass, VkExtent2D size)
{
	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0 };

	//Shader stages
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	// Setup shader stages
	createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	createInfo.pStages = shaderStages.data();

	//Vertex Input State
	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0 };
	{
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions_v
		{
			{ 0, sizeof(bhWorldVertex), VK_VERTEX_INPUT_RATE_VERTEX }
		};
		vertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions_v.size());
		vertexInputStateCI.pVertexBindingDescriptions = vertexInputBindingDescriptions_v.data();
	}
	createInfo.pVertexInputState = &vertexInputStateCI;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI =
	{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE
	};
	createInfo.pInputAssemblyState = &inputAssemblyStateCI;
	//createInfo.pTessellationState = ;

	VkPipelineViewportStateCreateInfo viewportStateCI = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0 };
	{
		VkViewport viewport = { float(0), float(size.height), float(size.width), -float(size.height), 0.f, 1.f }; 	// See VK_KHR_maintenance1 spec for negated height
		viewportStateCI.viewportCount = 1;
		viewportStateCI.pViewports = &viewport;

		VkRect2D scissor = { { 0, 0 }, { size.width, size.height } };
		viewportStateCI.scissorCount = 1;
		viewportStateCI.pScissors = &scissor;
	}
	createInfo.pViewportState = &viewportStateCI;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCI =
	{
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0,
		VK_FALSE, //depthClampEnable
		VK_FALSE, //rasterizerDiscardEnable
		VK_POLYGON_MODE_FILL, //polygonMode
		VK_CULL_MODE_BACK_BIT, //cullMode
		VK_FRONT_FACE_COUNTER_CLOCKWISE, //frontFace
		VK_FALSE, //depthBiasEnable
		0.f, //depthBiasConstantFactor
		0.f, //depthBiasClamp
		0.f, //depthBiasSlopeFactor
		1.f //lineWidth
	};
	createInfo.pRasterizationState = &rasterizationStateCI;

	VkPipelineMultisampleStateCreateInfo multisampleStateCI =
	{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0,
		VK_SAMPLE_COUNT_1_BIT, //rasterizationSamples
		VK_FALSE, //sampleShadingEnable
	};
	createInfo.pMultisampleState = &multisampleStateCI;

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI =
	{
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0,
		VK_TRUE, //depthTestEnable
		VK_TRUE, //depthWriteEnable
		VK_COMPARE_OP_LESS, //depthCompareOp
		VK_FALSE, //depthBoundsTestEnable
		VK_FALSE, //stencilTestEnable
	};
	createInfo.pDepthStencilState = &depthStencilStateCI;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCI = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0 };
	{
		colorBlendStateCI.logicOpEnable = VK_FALSE;
		colorBlendStateCI.logicOp = VK_LOGIC_OP_MAX_ENUM; //as long as logicOpEnable is VK_FALSE, we don't care about this

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		//colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		//colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		//colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		//colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		//colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		//colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &colorBlendAttachmentState;
	}
	createInfo.pColorBlendState = &colorBlendStateCI;
	//createInfo.pDynamicState = ;

	layout = CreateLayout();
	createInfo.layout = layout;
	createInfo.renderPass = renderPass;
	//createInfo.subpass = ;
	//createInfo.basePipelineHandle = ;
	//createInfo.basePipelineIndex = ;

	pipeline = device.CreateGraphicsPipeline(createInfo);
	return pipeline != VK_NULL_HANDLE ? VK_TRUE : VK_FALSE;
}

void bhPipeline::Destroy()
{
	device.DestroyPipeline(pipeline);
	device.DestroyPipelineLayout(layout);
}

//void bhPipeline::BindGraphics(VkCommandBuffer cmdBuffer)
//{
//	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
//}
