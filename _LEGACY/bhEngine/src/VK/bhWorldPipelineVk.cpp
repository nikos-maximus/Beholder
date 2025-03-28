#include <tinyxml2.h>
#include "VK/bhWorldPipelineVk.hpp"
#include "VK/bhDeviceVk.hpp"
#include "VK/bhTextureCacheVk.hpp"
#include "Mesh/bhMesh.hpp"
#include "bhLog.h"
#include "Platform/bhPlatform.hpp"
#include "bhImage.hpp"

namespace bhVk
{
  ////////////////////////////////////////////////////////////////////////////////
  VkPipelineLayout WorldPipeline::s_layout{ VK_NULL_HANDLE };
  VkDescriptorSetLayout WorldPipeline::s_descSetLayouts_v[NUM_DESC_SETS] = {};

  VkPipelineLayout GetPipelineLayout()
  {
    return WorldPipeline::s_layout;
  }

  static const char* g_SamplerIdxNames[WorldPipeline::Material::SamplerIdx::NUM_SAMPLERS] =
  {
    "albedo",
    "normalspecular"
  };

  bool CreateLayout(const RenderDevice& rd)
  {
    if (WorldPipeline::WorldPipeline::s_layout == VK_NULL_HANDLE)
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

      vkCreateDescriptorSetLayout(rd.device, &viewProjectionCI, rd.alloc, &(WorldPipeline::s_descSetLayouts_v[WorldPipeline::DESC_SET_VIEW_PROJECTION]));

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
      vkCreateDescriptorSetLayout(rd.device, &texturesCI, rd.alloc, &(WorldPipeline::s_descSetLayouts_v[WorldPipeline::DESC_SET_TEXTURE]));

      VkPipelineLayoutCreateInfo plCI =
      {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0,
        (uint32_t)WorldPipeline::NUM_DESC_SETS,
        WorldPipeline::s_descSetLayouts_v,
        (uint32_t)pushConstantRanges_v.size(),
        pushConstantRanges_v.data()
      };

      WorldPipeline::WorldPipeline::s_layout = VK_NULL_HANDLE;
      vkCreatePipelineLayout(rd.device, &plCI, rd.alloc, &WorldPipeline::s_layout); // TODO: Error check
      if (WorldPipeline::s_layout == VK_NULL_HANDLE)
      {
        return false;
      }
    }
    return true;
  }

  void DestroyLayout(const RenderDevice& rd)
  {
    if (WorldPipeline::s_layout == VK_NULL_HANDLE) { return; }

    vkDestroyPipelineLayout(rd.device, WorldPipeline::WorldPipeline::s_layout, rd.alloc);
    for (auto dsl = 0; dsl < WorldPipeline::NUM_DESC_SETS; ++dsl)
    {
      vkDestroyDescriptorSetLayout(rd.device, WorldPipeline::s_descSetLayouts_v[dsl], rd.alloc);
    }
  }

  WorldPipeline* WorldPipeline::Create(const RenderDevice& rd, uint32_t w, uint32_t h)
  {
    if (!CreateLayout(rd)) { return nullptr; }

    VkGraphicsPipelineCreateInfo pipelineCI = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    ////////////////////////////////////////////////////////////////////////////////
    // Vertex Input State
    static constexpr uint8_t STAGE_COUNT = 2;
    const char* shaderNames_v[STAGE_COUNT] =
    {
      "World.vert",
      "World.frag"
    };

    VkPipelineShaderStageCreateInfo shaderStageCI_v[STAGE_COUNT] = {};

    VkPipelineShaderStageCreateInfo& s0 = shaderStageCI_v[0];
    s0.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    s0.stage = GetShaderStage(SHADER_STG_VERTEX);
    s0.module = CreateShaderFromFile(rd, shaderNames_v[0]);
    s0.pName = "main";

    VkPipelineShaderStageCreateInfo& s1 = shaderStageCI_v[1];
    s1.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    s1.stage = GetShaderStage(SHADER_STG_FRAGMENT);
    s1.module = CreateShaderFromFile(rd, shaderNames_v[1]);
    s1.pName = "main";

    pipelineCI.stageCount = STAGE_COUNT;
    pipelineCI.pStages = shaderStageCI_v;

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions_v
    {
      { 0, sizeof(bhMeshVertex_t), VK_VERTEX_INPUT_RATE_VERTEX },
    };

    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions_v
    {
      { (uint32_t)MESH_BINDING_POSITIONS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex_t, bhMeshVertex_t::position) },
      { (uint32_t)MESH_BINDING_NORMALS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex_t, bhMeshVertex_t::normal) },
      { (uint32_t)MESH_BINDING_TANGENTS,	0,	VK_FORMAT_R32G32B32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex_t, bhMeshVertex_t::tangent) },
      { (uint32_t)MESH_BINDING_UV_0,		0,	VK_FORMAT_R32G32_SFLOAT,	(uint32_t)offsetof(bhMeshVertex_t, bhMeshVertex_t::uv_0) },
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
    //pipelineCI.pTessellationState = nullptr;

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
    assert(WorldPipeline::s_layout != VK_NULL_HANDLE);
    pipelineCI.layout = WorldPipeline::s_layout;

    ////////////////////////////////////////////////////////////////////////////////
    // Renderpass
    pipelineCI.renderPass = rd.presentRenderPass; // TODO: Must generalize in order to render to texture (for example)
    pipelineCI.subpass = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // Base WorldPipeline
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCI.basePipelineIndex = 0;

    ////////////////////////////////////////////////////////////////////////////////
    WorldPipeline* newPl = new WorldPipeline();
    VkResult result = vkCreateGraphicsPipelines(rd.device, VK_NULL_HANDLE, 1, &pipelineCI, rd.alloc, &(newPl->pipeline));

    ////////////////////////////////////////////////////////////////////////////////
    // Destroy creation objects
    for (auto sh = 0; sh < STAGE_COUNT; ++sh)
    {
      vkDestroyShaderModule(rd.device, shaderStageCI_v[sh].module, rd.alloc);
    }

    // Setup descriptor sets
    VkDescriptorSetAllocateInfo descriptorSetAI =
    {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
      rd.descriptorPool,
      (uint32_t)WorldPipeline::NUM_DESC_SETS,
      WorldPipeline::s_descSetLayouts_v
    };
    if (vkAllocateDescriptorSets(rd.device, &descriptorSetAI, newPl->descSets_v) != VK_SUCCESS)
    {
      // TODO: Report error
      assert(false);
    }

    // Setup camera memory
    newPl->cameraVPBuffer = CreateBuffer(rd, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
      sizeof(glm::mat4) + sizeof(glm::mat4)); // View + projection mat
    {
      VkDescriptorBufferInfo bufferInfo = { newPl->cameraVPBuffer.buffer, 0, VK_WHOLE_SIZE };
      VkWriteDescriptorSet writeDescSet =
      {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
        newPl->descSets_v[WorldPipeline::DESC_SET_VIEW_PROJECTION],
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr,
        &bufferInfo,
        nullptr
      };
      vkUpdateDescriptorSets(rd.device, 1, &writeDescSet, 0, nullptr);
    }

    VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    //newPl->vb = CreateBuffer(rd, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, propertyFlags, sizeof(bhMeshVertex) * WorldPipeline::MAX_VERTS);
    //newPl->ib = CreateBuffer(rd, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, propertyFlags, sizeof(bhMeshIdx_t) * WorldPipeline::MAX_INDS);

    return newPl;
  }

  void WorldPipeline::Destroy(const RenderDevice& rd, WorldPipeline*& pl)
  {
    //rd.DestroyBuffer(pl->ib);
    //rd.DestroyBuffer(pl->vb);
    DestroyBuffer(rd, pl->cameraVPBuffer);
    vkDestroyPipeline(rd.device, pl->pipeline, rd.alloc);
    delete pl;
    pl = nullptr;

    DestroyLayout(rd);
  }

  void WorldPipeline::Bind(const RenderDevice& rd, const WorldPipeline& pipeline)
  {
    VkCommandBuffer currCB = rd.commandBuffers_v[rd.swapChain.swapChainIdx];
    vkCmdBindPipeline(currCB, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

    vkCmdBindDescriptorSets(
      currCB,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      WorldPipeline::s_layout,
      WorldPipeline::DESC_SET_VIEW_PROJECTION,
      1,
      &(pipeline.descSets_v[WorldPipeline::DESC_SET_VIEW_PROJECTION]),
      0,
      nullptr);

    VkDeviceSize offsets[] = { 0 };
    //vkCmdBindVertexBuffers(currCB, 0, 1, &(pipeline->vb.buffer), offsets);
    //vkCmdBindIndexBuffer(currCB, pipeline->ib.buffer, 0, BH_MESH_INDEX_TYPE);
  }

  void WorldPipeline::SetCameraView(const RenderDevice& rd, const WorldPipeline& pipeline, const glm::mat4& view, const glm::mat4& proj)
  {
    CopyDataToBuffer(rd, pipeline.cameraVPBuffer, 0, sizeof(glm::mat4), &view);
    CopyDataToBuffer(rd, pipeline.cameraVPBuffer, sizeof(glm::mat4), sizeof(glm::mat4), &proj);
  }

  WorldPipeline::Material* WorldPipeline::Material::Create(const RenderDevice& rd, const Material::CreateInfo& mci)
  {
    WorldPipeline::Material* newMat = new WorldPipeline::Material();

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
    {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
      rd.descriptorPool,
      1,
      &(WorldPipeline::s_descSetLayouts_v[WorldPipeline::DESC_SET_TEXTURE])
    };

    if (vkAllocateDescriptorSets(rd.device, &descriptorSetAllocateInfo, &(newMat->texturesDescriptorSet)) != VK_SUCCESS)
    {
      // TODO: Report error
      assert(false);
    }

    assert(newMat->texturesDescriptorSet != VK_NULL_HANDLE);
    bool texturesValid = true;
    for (uint8_t samplerIdx = 0; samplerIdx < WorldPipeline::Material::NUM_SAMPLERS; ++samplerIdx)
    {
      const char* texPath = bhPlatform::CreateResourcePath(bhPlatform::ResourceType::RT_TEXTURE, mci.textureNames[samplerIdx]);
      const Texture* tx = TextureCache::Get(texPath);
      if (!tx)
      {
        tx = TextureCache::New(texPath);
        bhImage* txImage = bhImage::CreateFromFile(texPath, 4);
        if (txImage)
        {
          CreateTextureFromImage(rd, *(const_cast<Texture*>(tx)), *txImage, 0, 0);
          bhImage::Free(txImage);
        }
      }
      newMat->textureInputs_v[samplerIdx] = SetupTextureDescriptor(rd, *tx);
      texturesValid &= bhVk::IsTextureInputValid(newMat->textureInputs_v[samplerIdx]);
    }
    //{
    //  const Texture* tx = TextureCache::Get(mci.textureNames[WorldPipeline::Material::SAMPLER_ALBEDO]);
    //  if (!tx)
    //  {
    //    tx = TextureCache::New(mci.textureNames[WorldPipeline::Material::SAMPLER_ALBEDO]);
    //    bhImage* txImage = bhImage::CreateFromFile(mci.textureNames[WorldPipeline::Material::SAMPLER_ALBEDO], 4);
    //    if (txImage)
    //    {
    //      CreateTextureFromImage(rd, *(const_cast<Texture*>(tx)), *txImage, 0, 0);
    //      bhImage::Free(txImage);
    //    }
    //  }
    //  newMat->textureInputs_v[WorldPipeline::Material::SAMPLER_ALBEDO] = SetupTextureDescriptor(rd, *tx);
    //  texturesValid &= bhVk::IsTextureInputValid(newMat->textureInputs_v[WorldPipeline::Material::SAMPLER_ALBEDO]);
    //}
    //{
    //  const Texture* tx = TextureCache::Get(mci.textureNames[WorldPipeline::Material::SAMPLER_NORMAL_SPECULAR]);
    //  newMat->textureInputs_v[WorldPipeline::Material::SAMPLER_NORMAL_SPECULAR] = SetupTextureDescriptor(rd, *tx);
    //  texturesValid &= bhVk::IsTextureInputValid(newMat->textureInputs_v[WorldPipeline::Material::SAMPLER_NORMAL_SPECULAR]);
    //}
    if (!texturesValid)
    {
      return nullptr;
    }

    VkWriteDescriptorSet writeDescSet =
    {
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
      newMat->texturesDescriptorSet,
      0,
      0,
      WorldPipeline::Material::NUM_SAMPLERS,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      newMat->textureInputs_v
    };
    vkUpdateDescriptorSets(rd.device, 1, &writeDescSet, 0, nullptr);
    return newMat;
  }

  void WorldPipeline::Material::Destroy(const RenderDevice& rd, const WorldPipeline::Material* mat)
  {
    for (uint16_t s = 0; s < WorldPipeline::Material::NUM_SAMPLERS; ++s)
    {
      vkDestroySampler(rd.device, mat->textureInputs_v[s].sampler, rd.alloc);
      vkDestroyImageView(rd.device, mat->textureInputs_v[s].imageView, rd.alloc);
    }
    // In order to allow individual release of descriptor sets
    // we must create the descriptor pool with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT flag
    //vkFreeDescriptorSets(device, descriptorPool, 1, &mat->textureDescriptorsSet);
  }

  void WorldPipeline::Material::Bind(const RenderDevice& rd, const WorldPipeline::Material& mat)
  {
    VkCommandBuffer currCB = rd.commandBuffers_v[rd.swapChain.swapChainIdx];
    vkCmdBindDescriptorSets(
      currCB, VK_PIPELINE_BIND_POINT_GRAPHICS, WorldPipeline::s_layout,
      WorldPipeline::DESC_SET_TEXTURE, 1, &(mat.texturesDescriptorSet), 0, nullptr);
  }

  WorldPipeline::Material* WorldPipeline::Material::Load(const RenderDevice& rd, const char* fileName)
  {
    const char* filePath = bhPlatform::CreateResourcePath(bhPlatform::RT_MATERIAL, fileName);

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError result = doc.LoadFile(filePath);
    if (result != tinyxml2::XML_SUCCESS)
    {
      // TODO: Log error
      bhLog_Message(bhLogPriority::LP_ERROR, "Could not open material file %s", filePath);
      return nullptr;
    }
    auto rootEl = doc.RootElement();
    if (rootEl)
    {
      bool result = true;
      Material::CreateInfo mci;
      for (int si = 0; si < WorldPipeline::Material::SamplerIdx::NUM_SAMPLERS; ++si)
      {
        auto samplerEl = rootEl->FirstChildElement(g_SamplerIdxNames[si]);
        if (!samplerEl)
        {
          bhLog_Message(bhLogPriority::LP_ERROR, "WorldPipeline::Material file %s: %s element not provided", filePath, g_SamplerIdxNames[si]);
          result = false;
        }
        auto fileAttr = samplerEl->Attribute("file");
        if (!fileAttr || strlen(fileAttr) == 0)
        {
          bhLog_Message(bhLogPriority::LP_ERROR, "WorldPipeline::Material file %s: %s element does not provide a vaild file", filePath, g_SamplerIdxNames[si]);
          result = false;
        }
        mci.textureNames[si] = fileAttr;
      }
      if (result)
      {
        return Create(rd, mci);
      }
    }
    return nullptr;
  }

} // bhVk
