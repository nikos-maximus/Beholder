#include "VK/bhPipelineVK.hpp"
#include "VK/bhDeviceVK.hpp"
#include "bhImage.h"
#include "bhUtil.h"
#include "bhEnv.h"
#include "bhMesh.hpp"
#include "bhWindow.h"

#if BH_USE_MESH_INDEX_TYPE_UNIT16
#define BH_MESH_INDEX_TYPE VK_INDEX_TYPE_UINT16
#else
#define BH_MESH_INDEX_TYPE VK_INDEX_TYPE_UINT32
#endif

////////////////////////////////////////////////////////////////////////////////
bool bhWorldMaterial_IsTextureInputValid(const VkDescriptorImageInfo* dii)
{
	return (dii->sampler != VK_NULL_HANDLE) && (dii->imageView != VK_NULL_HANDLE); /*&& (dii->imageLayout != VK_IMAGE_LAYOUT_UNDEFINED);*/
}

////////////////////////////////////////////////////////////////////////////////
bool bhDeviceVK::IsPipelineValid_World(bhResourceID pipelineID)
{
	bhPipelineVK_World* pipeline = static_cast<bhPipelineVK_World*>(pipelineID);
	return pipeline->pipeline != VK_NULL_HANDLE; //Maybe we need more conditions?
}

VkSampler bhDeviceVK::bhWorldMaterial_CreateSampler() const
{
	float reqAnisotropy = GetAnisotropy();

	VkSamplerCreateInfo samplerCI =
	{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0,
		VK_FILTER_LINEAR, //magFilter
		VK_FILTER_LINEAR, //minFilter
		VK_SAMPLER_MIPMAP_MODE_LINEAR, //mipmapMode
		VK_SAMPLER_ADDRESS_MODE_REPEAT, //addressModeU
		VK_SAMPLER_ADDRESS_MODE_REPEAT, //addressModeV
		VK_SAMPLER_ADDRESS_MODE_REPEAT, //addressModeW
		0.f, //mipLodBias
		(reqAnisotropy > 0.f) ? VK_TRUE : VK_FALSE, //anisotropyEnable
		reqAnisotropy, //maxAnisotropy
		VK_FALSE, //compareEnable
		VK_COMPARE_OP_ALWAYS, //compareOp
		0.f, //minLod
		FLT_MAX, //maxLod
		VK_BORDER_COLOR_INT_OPAQUE_BLACK, //borderColor
		VK_FALSE //unnormalizedCoordinates
	};

	return CreateSampler(&samplerCI);
}

VkImageView bhDeviceVK::bhWorldMaterial_CreateTextureView(const bhImage* img, const bhTextureVK* texture) const
{
	VkFormat fmt = bhDeviceVK::PickImageFormat(img);
	if (fmt == VK_FORMAT_UNDEFINED)
	{
		return VK_NULL_HANDLE;
	}

	VkComponentMapping componentMapping = {};

	VkImageSubresourceRange imageSubresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
		0, //baseMipLevel
		bhImage_GetRequiredMipLevels(img), //levelCount
		0, //baseArrayLayer
		1 //layerCount
	};

	VkImageViewCreateInfo imageViewCI =
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
		texture->image,
		VK_IMAGE_VIEW_TYPE_2D,
		fmt,
		componentMapping,
		imageSubresourceRange
	}; 

	VkImageView imageView = VK_NULL_HANDLE;
	vkCreateImageView(device, &imageViewCI, deviceAllocator, &imageView);
	return imageView;
}

VkDescriptorImageInfo bhDeviceVK::bhWorldMaterial_SetupTextureInput(const std::string* textureName)
{
	char* path = bhUtil_CreatePath(bhEnv_GetEnvString(ENV_TEXTURES_PATH), textureName->c_str());
	bhImage* img = bhImage_CreateFromFile(path, 4);

	if (img == nullptr)
	{
		bhLog_Message(LOG_TYPE_ERROR, "Could not load image %s", path);
		bhUtil_FreePath(&path);
		return {};
	}

	if (img->retCode == 1)
	{
		bhLog_Message(LOG_TYPE_WARNING, "Loading image %s: Runtime conversion", path);
	}
	bhUtil_FreePath(&path);
	bhResourceID newTexID = CreateTextureFromImage(img, textureName->c_str(), 0, 0);
	assert(newTexID != BH_INVALID_RESOURCE);

	const bhTextureVK* newTexture = textureManager.GetResourceObject(newTexID);
	VkDescriptorImageInfo descriptorImgInfo = {};
	descriptorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImgInfo.imageView = bhWorldMaterial_CreateTextureView(img, newTexture);
	descriptorImgInfo.sampler = bhWorldMaterial_CreateSampler();

	return descriptorImgInfo;
}

bhResourceID bhDeviceVK::CreateMaterial_World(bhResourceID pipelineID, const bhWorldMaterialCreateInfo* materialCI)
{
	bhMaterialVK_World* outMaterial = new bhMaterialVK_World();
	outMaterial->pipeline = static_cast<bhPipelineVK_World*>(pipelineID);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
		descriptorPool,
		1,
		& (outMaterial->pipeline->descSetLayouts_v[bhPipelineVK_World::BINDING_SET_TEXTURES])
	};

	if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &(outMaterial->textureDescriptorsSet)) != VK_SUCCESS)
	{
		// TODO: Report error
		assert(false);
	}

	assert(outMaterial->textureDescriptorsSet != VK_NULL_HANDLE);
	bool texturesValid = true;

	VkDescriptorImageInfo* input_Albedo = &(outMaterial->textureInputs_v[SAMPLER_SLOT_ALBEDO]);
	*input_Albedo = bhWorldMaterial_SetupTextureInput(&materialCI->texture_Albedo);
	texturesValid &= bhWorldMaterial_IsTextureInputValid(input_Albedo);

	VkDescriptorImageInfo* input_NS = &(outMaterial->textureInputs_v[SAMPLER_SLOT_NORMAL_SPECULAR]);
	*input_NS = bhWorldMaterial_SetupTextureInput(&materialCI->texture_NormalSpecular);
	texturesValid &= bhWorldMaterial_IsTextureInputValid(input_NS);

	if (texturesValid)
	{
		VkWriteDescriptorSet writeDescSet =
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
			outMaterial->textureDescriptorsSet,
			0,
			0,
			NUM_SAMPLER_SLOTS,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			outMaterial->textureInputs_v
		};
		vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);
	}

	return outMaterial;
}

void bhDeviceVK::DestroyMaterial_World(bhResourceID materialID)
{
	bhMaterialVK_World* material = static_cast<bhMaterialVK_World*>(materialID);

	DestroySampler(material->textureInputs_v[SAMPLER_SLOT_ALBEDO].sampler);
	vkDestroyImageView(device, material->textureInputs_v[SAMPLER_SLOT_ALBEDO].imageView, deviceAllocator);

	DestroySampler(material->textureInputs_v[SAMPLER_SLOT_NORMAL_SPECULAR].sampler);
	vkDestroyImageView(device, material->textureInputs_v[SAMPLER_SLOT_NORMAL_SPECULAR].imageView, deviceAllocator);
}

////////////////////////////////////////////////////////////////////////////////
bool bhDeviceVK::bhWorldPipeline_CreateLayout(bhPipelineVK_World* outPipeline)
{
	////////////////////////////////////////////////////////////////////////////////
	// View/Projection
	std::vector<VkDescriptorSetLayoutBinding> viewProjectionBindings_v
	{
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
	};
	VkDescriptorSetLayoutCreateInfo viewProjectionCI = 
	{ 
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 
		(uint32_t)viewProjectionBindings_v.size(),
		viewProjectionBindings_v.data()
	};
	vkCreateDescriptorSetLayout(device, &viewProjectionCI, deviceAllocator, &(outPipeline->descSetLayouts_v[bhPipelineVK_World::BINDING_SET_VIEW]));

#if PIPELINE_MODEL
	////////////////////////////////////////////////////////////////////////////////
	// Model
	std::vector<VkDescriptorSetLayoutBinding> modelBindings_v
	{
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
	};
	VkDescriptorSetLayoutCreateInfo modelCI =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
		(uint32_t)modelBindings_v.size(),
		modelBindings_v.data()
	};
	vkCreateDescriptorSetLayout(device, &modelCI, deviceAllocator, &(outPipeline->descSetLayouts_v[bhPipelineVK_World::BINDING_SET_MODEL]));
#endif

	////////////////////////////////////////////////////////////////////////////////
	// Push Constants
	std::vector<VkPushConstantRange> pushConstantRanges_v =
	{
		{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) }
	};

	////////////////////////////////////////////////////////////////////////////////
	// Textures
	std::vector<VkDescriptorSetLayoutBinding> textureBindings_v
	{
		{ (uint32_t)SAMPLER_SLOT_ALBEDO, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ (uint32_t)SAMPLER_SLOT_NORMAL_SPECULAR, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
	};
	VkDescriptorSetLayoutCreateInfo texturesCI = 
	{ 
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 
		(uint32_t)textureBindings_v.size(), 
		textureBindings_v.data() 
	};
	vkCreateDescriptorSetLayout(device, &texturesCI, deviceAllocator, &(outPipeline->descSetLayouts_v[bhPipelineVK_World::BINDING_SET_TEXTURES]));

	////////////////////////////////////////////////////////////////////////////////
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = 
	{ 
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 
		bhPipelineVK_World::NUM_BINDING_SETS,
		outPipeline->descSetLayouts_v,
		(uint32_t)pushConstantRanges_v.size(),
		pushConstantRanges_v.data()
	};
	VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, deviceAllocator, &(outPipeline->layout));
	if(result != VK_SUCCESS)
	{
		// TODO: Handle error
		return false;
	}
	return true;

	//VkDescriptorSetAllocateInfo descriptorSetAI = VkDescriptorSetAllocateInfo_Init(descriptorPool);
	//descriptorSetAI.descriptorSetCount = descSetLayouts_v.size();
	//descriptorSetAI.pSetLayouts = descSetLayouts_v.data();

	//pipeline.descriptorSets_v.resize(descSetLayouts_v.size());
	//auto result = vkAllocateDescriptorSets(device, &descriptorSetAI, pipeline.descriptorSets_v.data());

	// [Label_01] see comments in DestroyPipeline
}

void bhDeviceVK::bhWorldPipeline_DestroyDataBuffers(bhPipelineVK_World* pipeline)
{
	//bhVK_DestroyBuffer(deviceVK, &(pipeline->dynamicModelBuffer));
	//bhUtil_AlignedFree(pipeline->modelData);
	//pipeline->modelData = nullptr;

	DestroyBuffer(&(pipeline->indexBuffer));
	DestroyBuffer(&(pipeline->vertexBuffer));
}

void bhDeviceVK::DestroyPipeline_World(bhResourceID pipelineID)
{
	bhPipelineVK_World* pipeline = static_cast<bhPipelineVK_World*>(pipelineID);
	bhWorldPipeline_DestroyDataBuffers(pipeline);
	
	// Destroy viewProjection object
	DestroyBuffer(&(pipeline->viewDataBuffer));

	// NOTE
	// vkDestroyDescriptorSetLayout loop was moved here from [Label_01] in CreatePipelineLayout, as it didn't work with integrated GPU
	// The possible reason is that with shared memory, we need to keep the descSetLayouts_v alive as they are referenced while updating descriptor sets
	// Apparently, deleting at [Label_01] works fine with a discrete GPU (layout data is uploaded to GPU memory??)
	// Might be a way to sove same memory if we know we are working with discrete GPU?
	for (int ls = 0; ls < bhPipelineVK_World::NUM_BINDING_SETS; ++ls)
	{
		vkDestroyDescriptorSetLayout(device, pipeline->descSetLayouts_v[ls], deviceAllocator);
	}

	//vkFreeDescriptorSets(device, pipeline.descriptorPool, pipeline.numSetLayouts, pipeline.descriptorSets_v);
	// Not using VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT when creating descriptor pool
	vkDestroyPipeline(device, pipeline->pipeline, deviceAllocator);
	vkDestroyPipelineLayout(device, pipeline->layout, deviceAllocator);
	pipeline = {};
}

void bhDeviceVK::UsePipeline_World(bhResourceID pipelineID)
{
	bhPipelineVK_World* pipeline = static_cast<bhPipelineVK_World*>(pipelineID);
	vkCmdBindPipeline(commandBuffers_v[swapChain.swapChainIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
}

void bhDeviceVK::SetCameraView_World(bhResourceID pipelineID, const bhCamera::ViewData* viewProjection)
{
	bhPipelineVK_World* pipeline = static_cast<bhPipelineVK_World*>(pipelineID);
	CopyDataToBuffer(&(pipeline->viewDataBuffer), 0, sizeof(bhCamera::ViewData), viewProjection);
	vkCmdBindDescriptorSets(commandBuffers_v[swapChain.swapChainIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout,
		bhPipelineVK_World::BINDING_SET_VIEW, 1, &(pipeline->descSets_v[bhPipelineVK_World::BINDING_SET_VIEW]), 0, nullptr);
}

void bhDeviceVK::UseMaterial_World(bhResourceID materialID)
{
	bhMaterialVK_World* mat = static_cast<bhMaterialVK_World*>(materialID);
	vkCmdBindDescriptorSets(commandBuffers_v[swapChain.swapChainIdx], VK_PIPELINE_BIND_POINT_GRAPHICS,
		mat->pipeline->layout, bhPipelineVK_World::BINDING_SET_TEXTURES, 1, &(mat->textureDescriptorsSet), 0, nullptr);
}

void bhDeviceVK::bhWorldPipeline_CreateDataBuffers(bhPipelineVK_World* pipeline)
{
	VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	pipeline->vertexBuffer = CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, propertyFlags, sizeof(bhWorldVertex) * BH_BUFFER_MAX_VERTS);
	pipeline->indexBuffer = CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, propertyFlags, sizeof(bhMeshIdx_t) * BH_BUFFER_MAX_INDS);

	//VkPhysicalDeviceProperties pdProps;
	//vkGetPhysicalDeviceProperties(deviceVK->physicalDevice, &pdProps);
	//pipeline->dynamicAlignment = bhMath_Max(sizeof(bhModelData), pdProps.limits.minUniformBufferOffsetAlignment);

	//pipeline->modelData = (bhModelData*)bhUtil_AlignedAlloc(sizeof(bhModelData) * BH_MAX_INSTANCES, pipeline->dynamicAlignment);
	//propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	//pipeline->dynamicModelBuffer = bhVK_CreateBuffer(deviceVK, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, propertyFlags, sizeof(bhModelData) * BH_MAX_INSTANCES);
}

bhResourceID bhDeviceVK::CreatePipeline_World()
{
	std::vector<std::string> shaderNames_v{ "World.vert","World.frag" };

	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions_v
	{
		{ 0, sizeof(bhWorldVertex), VK_VERTEX_INPUT_RATE_VERTEX },
	};
	vertexInputStateCI.vertexBindingDescriptionCount = (uint32_t)vertexInputBindingDescriptions_v.size();
	vertexInputStateCI.pVertexBindingDescriptions = vertexInputBindingDescriptions_v.data();

	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions_v
	{
		{ (uint32_t)MESH_BINDING_POSITIONS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhWorldVertex, bhWorldVertex::position) },
		{ (uint32_t)MESH_BINDING_NORMALS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhWorldVertex, bhWorldVertex::normal) },
		{ (uint32_t)MESH_BINDING_TANGENTS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhWorldVertex, bhWorldVertex::tangent) },
		{ (uint32_t)MESH_BINDING_UV_0,		0,	VK_FORMAT_R32G32_SFLOAT,	(uint32_t)offsetof(bhWorldVertex, bhWorldVertex::uv) },
	};
	vertexInputStateCI.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions_v.size();
	vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributeDescriptions_v.data();

	//TODO: Maybe we don't always construct the pipeline to render to full window
	int w = 0, h = 0;
	bhGetMainWindowSize(&w, &h);
	VkViewport viewport = { float(0), float(h), float(w), -float(h), 0.f, 1.f };
	// See VK_KHR_maintenance1 spec for negated height

	VkRect2D scissor = {};
	//scissor.offset.x = scissor.offset.y = 0;
	scissor.extent.width = w;
	scissor.extent.height = h;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	//colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	bhPipelineVK_World* outPipeline = new bhPipelineVK_World();
	bool layoutResult = bhWorldPipeline_CreateLayout(outPipeline);
	if (!layoutResult)
	{
		assert(false);
		return BH_INVALID_RESOURCE;
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo_v(shaderNames_v.size());
	for (uint32_t sh = 0; sh < shaderNames_v.size(); ++sh)
	{
		VkPipelineShaderStageCreateInfo& scInfo = shaderStageCreateInfo_v[sh];
		scInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		scInfo.pName = "main";
		//info.pSpecializationInfo = nullptr; ? Very VERY useful for future use

		if (!CreateShaderFromFile(shaderNames_v[sh].c_str(), &(scInfo.module), &(scInfo.stage)))
		{
			assert(false);
		}
	}

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = 
	{ 
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 
		VK_FALSE 
	};

	VkPipelineViewportStateCreateInfo viewportStateCI = 
	{ 
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 
		1, 
		&viewport, 
		1, 
		&scissor 
	};

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

	VkPipelineMultisampleStateCreateInfo multisampleStateCI = 
	{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0,
		VK_SAMPLE_COUNT_1_BIT, //rasterizationSamples
		VK_FALSE, //sampleShadingEnable
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = 
	{
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0,
		VK_TRUE, //depthTestEnable
		VK_TRUE, //depthWriteEnable
		VK_COMPARE_OP_LESS, //depthCompareOp
		VK_FALSE, //depthBoundsTestEnable
		VK_FALSE, //stencilTestEnable
	};

	VkPipelineColorBlendStateCreateInfo colorBlendStateCI =
	{
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0,
		VK_FALSE, //logicOpEnable
		VK_LOGIC_OP_MAX_ENUM, //logicOp - as long as logicOpEnable is VK_FALSE, we don't care about this
		1, //attachmentCount
		&colorBlendAttachmentState //pAttachments
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineCI = 
	{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0,
		(uint32_t)shaderNames_v.size(), //stageCount
		shaderStageCreateInfo_v.data(), //pStages
		&vertexInputStateCI, //pVertexInputState
		&inputAssemblyStateCI, //pInputAssemblyState
		nullptr,
		&viewportStateCI, //pViewportState
		&rasterizationStateCI, //pRasterizationState
		&multisampleStateCI, //pMultisampleState
		&depthStencilStateCI, //pDepthStencilState
		&colorBlendStateCI, //pColorBlendState
		nullptr, //pDynamicState
		outPipeline->layout, //layout
		renderPass, //renderPass
	};

	VkResult createResult = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCI, deviceAllocator, &(outPipeline->pipeline));
	if (createResult == VK_SUCCESS)
	{
		for (size_t sh = 0; sh < shaderStageCreateInfo_v.size(); ++sh)
		{
			vkDestroyShaderModule(device, shaderStageCreateInfo_v[sh].module, deviceAllocator);
		}

		// Init viewProjection object
		outPipeline->viewDataBuffer = CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(bhCamera::ViewData));

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
			descriptorPool,
			2,
			outPipeline->descSetLayouts_v
		};
		if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, outPipeline->descSets_v) != VK_SUCCESS)
		{
			// TODO: Report error
			assert(false);
		}

		{
			VkDescriptorBufferInfo bufferInfo = { outPipeline->viewDataBuffer.buffer, 0, VK_WHOLE_SIZE };
			VkWriteDescriptorSet writeDescSet =
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
				outPipeline->descSets_v[bhPipelineVK_World::BINDING_SET_VIEW],
				0,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfo,
				nullptr
			};
			vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);
		}

		bhWorldPipeline_CreateDataBuffers(outPipeline);
	#if PIPELINE_MODEL
		{
			VkDescriptorBufferInfo bufferInfo = { outPipeline->dynamicModelBuffer.buffer, 0, sizeof(bhModelData) };
			VkWriteDescriptorSet writeDescSet =
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
				outPipeline->descSets_v[bhPipelineVK_World::BINDING_SET_MODEL],
				0,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				nullptr,
				&bufferInfo,
				nullptr
			};
			vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);
		}
	#endif
	}
	return static_cast<bhResourceID>(outPipeline);
}

void bhDeviceVK::UploadMesh_World(bhResourceID pipelineID, bhResourceID meshID)
{
	bhMesh* mesh = static_cast<bhMesh*>(meshID);

	bhPipelineVK_World* pipeline = static_cast<bhPipelineVK_World*>(pipelineID);
	mesh->offsets.offsetsVK.vertexOffset = pipeline->vertexBufferWriteOffset;
	VkDeviceSize reqVertexSize = mesh->numVerts * sizeof(bhWorldVertex);
	pipeline->vertexBufferWriteOffset += reqVertexSize;
	CopyDataToBuffer(&(pipeline->vertexBuffer), mesh->offsets.offsetsVK.vertexOffset, reqVertexSize, mesh->verts);

	mesh->offsets.offsetsVK.indexOffset = pipeline->indexBufferWriteOffset;
	VkDeviceSize reqIdxSize = mesh->numInds * sizeof(bhMeshIdx_t);
	pipeline->indexBufferWriteOffset += reqIdxSize;
	CopyDataToBuffer(&(pipeline->indexBuffer), mesh->offsets.offsetsVK.indexOffset, reqIdxSize, mesh->inds);
}

void bhDeviceVK::RenderMesh_World(bhResourceID pipelineID, bhResourceID meshID, const glm::mat4* transform)
{
	bhPipelineVK_World* pipeline = static_cast<bhPipelineVK_World*>(pipelineID);
	bhMesh* mesh = static_cast<bhMesh*>(meshID);

	VkCommandBuffer commandBuffer = commandBuffers_v[swapChain.swapChainIdx];
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(pipeline->vertexBuffer.buffer), &(mesh->offsets.offsetsVK.vertexOffset));
	vkCmdBindIndexBuffer(commandBuffer, pipeline->indexBuffer.buffer, mesh->offsets.offsetsVK.indexOffset, BH_MESH_INDEX_TYPE);

	vkCmdPushConstants(commandBuffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), transform);

	vkCmdDrawIndexed(commandBuffer, mesh->numInds, 1, 0, 0, 0);
}

#if PIPELINE_MODEL
void RenderMesh_World(VkCommandBuffer commandBuffer, const bhPipelineVK_World* pipeline, const bhMeshVK* deviceMesh, uint32_t modelIdx)
{
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(pipeline->vertexBuffer.buffer), &(deviceMesh->vertexOffset));
	vkCmdBindIndexBuffer(commandBuffer, pipeline->indexBuffer.buffer, deviceMesh->indexOffset, BH_MESH_INDEX_TYPE);

	uint32_t dynamicOffset = modelIdx * static_cast<uint32_t>(pipeline->dynamicAlignment);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout,
		bhPipelineVK_World::BINDING_SET_MODEL, 1, &(pipeline->descSets_v[bhPipelineVK_World::BINDING_SET_MODEL]), 1, &dynamicOffset);

	vkCmdDrawIndexed(commandBuffer, deviceMesh->hostMesh.GetNumInds(), 1, 0, 0, 0);
}
#endif

#if PIPELINE_INDEXED
void RenderMesh_World(VkCommandBuffer commandBuffer, const bhPipelineVK_World* pipeline, const bhMeshVK* deviceMesh, uint32_t modelIdx)
{
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(pipeline->vertexBuffer.buffer), &(deviceMesh->vertexOffset));
	vkCmdBindVertexBuffers(commandBuffer, 1, 1, &(pipeline->dynamicModelBuffer.buffer), &instOffsets);
	vkCmdBindIndexBuffer(commandBuffer, pipeline->indexBuffer.buffer, deviceMesh->indexOffset, BH_MESH_INDEX_TYPE);

	vkCmdDrawIndexed(commandBuffer, deviceMesh->hostMesh.GetNumInds(), 1, 0, 0, 0);
}
#endif

//void bhWorldPipeline_UploadInstanceData(const bhPipelineVK_World* pipeline, const bhModelData* modelData, size_t numInstances)
//{
//	const bhDeviceVK* deviceVK = bhRenderer_GetRenderDevice();
//
//	uint32_t numRenderInstances = bhMath_Min((uint32_t)BH_MAX_INSTANCES, (uint32_t)numInstances);
//	// TODO : Bounds check
//	bhVK_CopyDataToBuffer(device, &(pipeline->dynamicModelBuffer), 0, sizeof(bhModelData) * numRenderInstances, modelData);
//}
