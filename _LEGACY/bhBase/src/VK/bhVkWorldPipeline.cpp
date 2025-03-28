#include <tinyxml2.h>
#include "VK/bhVkWorldPipeline.hpp"
#include "VK/bhVkRenderDevice.hpp"
#include "Mesh/bhMesh.hpp"
#include "Texture/bhTextureCache.hpp"
#include "bhLog.h"

namespace bhVk
{
	static const char* g_SamplerIdxNames[WorldPipeline::SamplerIdx::NUM_SAMPLERS] =
	{
		"albedo",
		"normalspecular"
	};

	VkPipelineLayout RenderDevice::CreateWorldPipelineLayout()
	{
		////////////////////////////////////////////////////////////////////////////////
		// Vertex Stage

		// View/Projection
		std::vector<VkDescriptorSetLayoutBinding> viewProjectionBindings_v
		{
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT }
		};
		VkDescriptorSetLayoutCreateInfo viewProjectionCI =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
			(uint32_t)viewProjectionBindings_v.size(),
			viewProjectionBindings_v.data()
		};
		vkCreateDescriptorSetLayout(device, &viewProjectionCI, allocator, &(worldPipeline.descSetLayouts_v[WorldPipeline::DESC_SET_VIEW_PROJECTION]));

		// Push Constants
		std::vector<VkPushConstantRange> pushConstantRanges_v =
		{
			{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) } // Model matrix (transform)
		};

		////////////////////////////////////////////////////////////////////////////////
		// Fragmet Stage

		// Textures
		std::vector<VkDescriptorSetLayoutBinding> textureBindings_v
		{
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
		};
		VkDescriptorSetLayoutCreateInfo texturesCI =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
			(uint32_t)textureBindings_v.size(),
			textureBindings_v.data()
		};
		vkCreateDescriptorSetLayout(device, &texturesCI, allocator, &(worldPipeline.descSetLayouts_v[WorldPipeline::DESC_SET_TEXTURE]));

		VkPipelineLayoutCreateInfo plCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0,
			(uint32_t)WorldPipeline::NUM_DESC_SETS,
			worldPipeline.descSetLayouts_v,
			(uint32_t)pushConstantRanges_v.size(),
			pushConstantRanges_v.data()
		};

		VkPipelineLayout pl = VK_NULL_HANDLE;
		vkCreatePipelineLayout(device, &plCI, allocator, &pl); // TODO: Error check
		return pl;
	}

	void RenderDevice::DestroyWorldPipelineLayout()
	{
		vkDestroyPipelineLayout(device, worldPipeline.layout, allocator);
		for (auto dsl = 0; dsl < WorldPipeline::NUM_DESC_SETS; ++dsl)
		{
			vkDestroyDescriptorSetLayout(device, worldPipeline.descSetLayouts_v[dsl], allocator);
		}
	}

	VkBool32 RenderDevice::CreateWorldPipeline(uint32_t w, uint32_t h)
	{
		VkGraphicsPipelineCreateInfo pipelineCI = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0 };

		////////////////////////////////////////////////////////////////////////////////
		// Vertex Input State
		std::vector<const char*> shaderNames_v
		{
			"World.vert",
			"World.frag"
		};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCI_v(shaderNames_v.size(), { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0 });
		for (auto sh = 0; sh < shaderStageCI_v.size(); ++sh)
		{
			VkPipelineShaderStageCreateInfo& ssCI = shaderStageCI_v[sh];
			ssCI.pName = "main";
			//info.pSpecializationInfo = nullptr; ? VERY useful for future use

			if (!CreateShaderStageFromFile(ssCI, shaderNames_v[sh], ssCI.pName))
			{
				return VK_FALSE;
			}
		}

		pipelineCI.stageCount = (uint32_t)shaderStageCI_v.size();
		pipelineCI.pStages = shaderStageCI_v.data();

		////////////////////////////////////////////////////////////////////////////////
		// Vertex Input State
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions_v
		{
			{ 0, sizeof(bhMeshVertex), VK_VERTEX_INPUT_RATE_VERTEX },
		};

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions_v
		{
			{ (uint32_t)MESH_BINDING_POSITIONS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex, bhMeshVertex::position) },
			{ (uint32_t)MESH_BINDING_NORMALS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex, bhMeshVertex::normal) },
			{ (uint32_t)MESH_BINDING_TANGENTS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex, bhMeshVertex::tangent) },
			{ (uint32_t)MESH_BINDING_UV_0,		0,	VK_FORMAT_R32G32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex, bhMeshVertex::uv_0) },
		};

		VkPipelineVertexInputStateCreateInfo vertexInputStateCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0,
			(uint32_t)vertexInputBindingDescriptions_v.size(),
			vertexInputBindingDescriptions_v.data(),
			(uint32_t)vertexInputAttributeDescriptions_v.size(),
			vertexInputAttributeDescriptions_v.data()
		};

		pipelineCI.pVertexInputState = &vertexInputStateCI;

		////////////////////////////////////////////////////////////////////////////////
		// Assembly State
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_FALSE
		};

		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;

		////////////////////////////////////////////////////////////////////////////////
		// Tesselation State
		pipelineCI.pTessellationState = nullptr;

		////////////////////////////////////////////////////////////////////////////////
		// Viewport State
		VkViewport viewport = { float(0), float(h), float(w), -float(h), 0.0f, 1.0f };
		// See VK_KHR_maintenance1 spec for negated height
		VkRect2D scissor = { { 0, 0 }, { w, h } };

		VkPipelineViewportStateCreateInfo viewportStateCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0,
			1,
			&viewport,
			1,
			&scissor
		};

		pipelineCI.pViewportState = &viewportStateCI;

		////////////////////////////////////////////////////////////////////////////////
		// Rasterization State
		VkPipelineRasterizationStateCreateInfo rasterizationStateCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0,
			VK_FALSE, //depthClampEnable
			VK_FALSE, //rasterizerDiscardEnable
			VK_POLYGON_MODE_FILL, //polygonMode
			VK_CULL_MODE_NONE,// VK_CULL_MODE_BACK_BIT, //cullMode
			VK_FRONT_FACE_COUNTER_CLOCKWISE, //frontFace
			VK_FALSE, //depthBiasEnable
			0.0f, //depthBiasConstantFactor
			0.0f, //depthBiasClamp
			0.0f, //depthBiasSlopeFactor
			1.0f //lineWidth
		};

		pipelineCI.pRasterizationState = &rasterizationStateCI;

		////////////////////////////////////////////////////////////////////////////////
		// Multisample State
		VkPipelineMultisampleStateCreateInfo multisampleStateCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0,
			VK_SAMPLE_COUNT_1_BIT, //rasterizationSamples
			VK_FALSE, //sampleShadingEnable
		};

		pipelineCI.pMultisampleState = &multisampleStateCI;

		////////////////////////////////////////////////////////////////////////////////
		// Depth Stencil State
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0,
			VK_TRUE, //depthTestEnable
			VK_TRUE, //depthWriteEnable
			VK_COMPARE_OP_LESS, //depthCompareOp
			VK_FALSE, //depthBoundsTestEnable
			VK_FALSE, //stencilTestEnable
		};

		pipelineCI.pDepthStencilState = &depthStencilStateCI;

		////////////////////////////////////////////////////////////////////////////////
		// Color Blend State
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		//colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		//colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		//colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		//colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		//colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		//colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI =
		{
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0,
			VK_FALSE, //logicOpEnable
			VK_LOGIC_OP_MAX_ENUM, //logicOp - as long as logicOpEnable is VK_FALSE, we don't care about this
			1, //attachmentCount
			&colorBlendAttachmentState //pAttachments
		};

		pipelineCI.pColorBlendState = &colorBlendStateCI;

		////////////////////////////////////////////////////////////////////////////////
		// Dynamic State
		pipelineCI.pDynamicState = nullptr;

		////////////////////////////////////////////////////////////////////////////////
		// Layout
		worldPipeline.layout = CreateWorldPipelineLayout(); // Create if not null?

		pipelineCI.layout = worldPipeline.layout;

		////////////////////////////////////////////////////////////////////////////////
		// Renderpass
		pipelineCI.renderPass = presentRenderPass; // TODO: Must generalize in order to render to texture (for example)
		pipelineCI.subpass = 0;

		////////////////////////////////////////////////////////////////////////////////
		// Base Pipeline
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCI.basePipelineIndex = 0;

		////////////////////////////////////////////////////////////////////////////////
		worldPipeline.pipeline = CreateGraphicsPipeline(pipelineCI);

		////////////////////////////////////////////////////////////////////////////////
		// Destroy creation objects
		for (auto sh = 0; sh < shaderStageCI_v.size(); ++sh)
		{
			vkDestroyShaderModule(device, shaderStageCI_v[sh].module, allocator);
		}
		if (!worldPipeline.pipeline)
		{
			return VK_FALSE;
		}

		// Setup descriptor sets
		VkDescriptorSetAllocateInfo descriptorSetAI =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
			descriptorPool,
			(uint32_t)WorldPipeline::NUM_DESC_SETS,
			worldPipeline.descSetLayouts_v
		};
		if (vkAllocateDescriptorSets(device, &descriptorSetAI, worldPipeline.descSets_v) != VK_SUCCESS)
		{
			// TODO: Report error
			assert(false);
		}

		// Setup camera memory
		worldPipeline.cameraVPBuffer = CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(bhCamera::ViewData));
		{
			VkDescriptorBufferInfo bufferInfo = { worldPipeline.cameraVPBuffer.buffer, 0, VK_WHOLE_SIZE };
			VkWriteDescriptorSet writeDescSet =
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
				worldPipeline.descSets_v[WorldPipeline::DESC_SET_VIEW_PROJECTION],
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

		VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		worldPipeline.vb = CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, propertyFlags, sizeof(bhMeshVertex) * WorldPipeline::MAX_VERTS);
		worldPipeline.ib = CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, propertyFlags, sizeof(bhMeshIdx_t) * WorldPipeline::MAX_INDS);

		return VK_TRUE;
	}

	void RenderDevice::DestroyWorldPipeline()
	{
		DestroyBuffer(worldPipeline.ib);
		DestroyBuffer(worldPipeline.vb);
		DestroyBuffer(worldPipeline.cameraVPBuffer);
		DestroyPipeline(worldPipeline.pipeline);
	}

	void RenderDevice::BindWorldPipeline()
	{
		auto currCmdBuffer = commandBuffers_v[swapChain.swapChainIdx];
		vkCmdBindPipeline(currCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, worldPipeline.pipeline);

		vkCmdBindDescriptorSets(
			currCmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			worldPipeline.layout,
			WorldPipeline::DESC_SET_VIEW_PROJECTION,
			1,
			&(worldPipeline.descSets_v[WorldPipeline::DESC_SET_VIEW_PROJECTION]),
			0,
			nullptr);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(currCmdBuffer, 0, 1, &(worldPipeline.vb.buffer), offsets);
		vkCmdBindIndexBuffer(currCmdBuffer, worldPipeline.ib.buffer, 0, BH_MESH_INDEX_TYPE);
	}

	void RenderDevice::SetWorldCameraView(const bhCamera::ViewData* vd)
	{
		CopyDataToBuffer(worldPipeline.cameraVPBuffer, 0, sizeof(bhCamera::ViewData), vd);
	}

	void RenderDevice::UploadWorldMesh(bhMesh* mesh)
	{
		if (mesh->deviceData.IsUploaded())
		{
			return;
		}
		mesh->deviceData.offsets.vertexOffset = worldPipeline.vertexOffset;
		worldPipeline.vertexOffset += mesh->GetNumVerts();
		VkDeviceSize reqVertexSize = mesh->GetNumVerts() * sizeof(bhMeshVertex);
		CopyDataToBuffer(worldPipeline.vb, worldPipeline.vbMemOffs, reqVertexSize, mesh->GetVerts());
		worldPipeline.vbMemOffs += reqVertexSize;

		mesh->deviceData.offsets.indexOffset = worldPipeline.indexOffset;
		worldPipeline.indexOffset += mesh->GetNumInds();
		VkDeviceSize reqIdxSize = mesh->GetNumInds() * sizeof(bhMeshIdx_t);
		CopyDataToBuffer(worldPipeline.ib, worldPipeline.ibMemOffs, reqIdxSize, mesh->GetInds());
		worldPipeline.ibMemOffs += reqIdxSize;

		mesh->deviceData.SetUploaded();
	}

	void RenderDevice::RenderWorldMesh(const bhMesh* mesh, const glm::mat4& transform)
	{
		auto currCmdBuffer = commandBuffers_v[swapChain.swapChainIdx];
		vkCmdPushConstants(currCmdBuffer, worldPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
		vkCmdDrawIndexed(currCmdBuffer, mesh->GetNumInds(), 1, mesh->deviceData.offsets.indexOffset, mesh->deviceData.offsets.vertexOffset, 0);
	}

	WorldPipeline::Material RenderDevice::CreateWorldMaterial(const WorldPipeline::MaterialCreateInfo& matCI) const
	{
		WorldPipeline::Material newMat;

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
			descriptorPool,
			1,
			&(worldPipeline.descSetLayouts_v[WorldPipeline::DESC_SET_TEXTURE])
		};

		if (vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &(newMat.texturesDescriptorSet)) != VK_SUCCESS)
		{
			// TODO: Report error
			assert(false);
		}

		assert(newMat.texturesDescriptorSet != VK_NULL_HANDLE);
		bool texturesValid = true;
		{
			auto txRes = bhTextureCache::Get()->GetTexture(matCI.textureNames[WorldPipeline::SAMPLER_ALBEDO]); //Load image
			auto txImg = txRes->GetResource()->GetImage();
			txRes->GetResource()->SetTexture(CreateTextureFromImage(txImg, 0, 0));
			newMat.textureInputs_v[WorldPipeline::SAMPLER_ALBEDO] = SetupTextureDescriptor(*txRes->GetResource());
			texturesValid &= IsTextureInputValid(newMat.textureInputs_v[WorldPipeline::SAMPLER_ALBEDO]);
		}
		{
			auto txRes = bhTextureCache::Get()->GetTexture(matCI.textureNames[WorldPipeline::SAMPLER_NORMAL_SPECULAR]); //Load image
			auto txImg = txRes->GetResource()->GetImage();
			txRes->GetResource()->SetTexture(CreateTextureFromImage(txImg, 0, 0));
			newMat.textureInputs_v[WorldPipeline::SAMPLER_NORMAL_SPECULAR] = SetupTextureDescriptor(*txRes->GetResource());
			texturesValid &= IsTextureInputValid(newMat.textureInputs_v[WorldPipeline::SAMPLER_NORMAL_SPECULAR]);
		}
		if (!texturesValid)
		{
			return {};
		}

		VkWriteDescriptorSet writeDescSet =
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
			newMat.texturesDescriptorSet,
			0,
			0,
			WorldPipeline::NUM_SAMPLERS,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			newMat.textureInputs_v
		};
		vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);
		return newMat;
	}

	void RenderDevice::DestroyWorldMaterial(const WorldPipeline::Material& mat) const
	{
		for (uint16_t s = 0; s < WorldPipeline::NUM_SAMPLERS; ++s)
		{
			vkDestroySampler(device, mat.textureInputs_v[s].sampler, allocator);
			vkDestroyImageView(device, mat.textureInputs_v[s].imageView, allocator);
		}
		// In order to allow individual release of descriptor sets
		// we must create the descriptor pool with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT flag
		//vkFreeDescriptorSets(device, descriptorPool, 1, &mat.textureDescriptorsSet);
	}

	void RenderDevice::BindWorldMaterial(const WorldPipeline::Material& mat)
	{
		vkCmdBindDescriptorSets(
			commandBuffers_v[swapChain.swapChainIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, worldPipeline.layout,
			WorldPipeline::DESC_SET_TEXTURE, 1, &(mat.texturesDescriptorSet), 0, nullptr);
	}

	WorldPipeline::Material RenderDevice::LoadWorldMaterial(const char* fileName) const
	{
		using namespace tinyxml2;

		const char* filePath = bhPlatform::CreateResourcePath(bhPlatform::RT_MATERIAL, fileName);

		XMLDocument doc;
		XMLError result = doc.LoadFile(filePath);
		if (result != XML_SUCCESS)
		{
			// TODO: Log error
			bhLog_Message(bhLogPriority::LP_ERROR, "Could not open material file %s", filePath);
			return {};
		}
		auto rootEl = doc.RootElement();
		if (rootEl)
		{
			bool result = true;
			WorldPipeline::MaterialCreateInfo mci;
			for (int si = 0; si < WorldPipeline::SamplerIdx::NUM_SAMPLERS; ++si)
			{
				auto samplerEl = rootEl->FirstChildElement(g_SamplerIdxNames[si]);
				if (!samplerEl)
				{
					bhLog_Message(bhLogPriority::LP_ERROR, "Material file %s: %s element not provided", filePath, g_SamplerIdxNames[si]);
					result = false;
				}
				auto fileAttr = samplerEl->Attribute("file");
				if (!fileAttr || strlen(fileAttr) == 0)
				{
					bhLog_Message(bhLogPriority::LP_ERROR, "Material file %s: %s element does not provide a vaild file", filePath, g_SamplerIdxNames[si]);
					result = false;
				}
				mci.textureNames[si] = fileAttr;
			}
			if (result)
			{
				return CreateWorldMaterial(mci);
			}
		}
		return {};
	}
}
