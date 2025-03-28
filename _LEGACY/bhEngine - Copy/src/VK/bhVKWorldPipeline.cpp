#include <tinyxml2.h>
#include <assert.h>
#include "VK/bhVKWorldPipeline.hpp"
#include "VK/bhVkRenderDevice.hpp"
#include "Mesh/bhMesh.hpp"
#include "bhLog.h"
#include "Platform/bhPlatform.hpp"

VkPipelineLayout bhVKWorldPipeline::s_layout{ VK_NULL_HANDLE };
VkDescriptorSetLayout bhVKWorldPipeline::s_descSetLayouts_v[NUM_DESC_SETS] = {};
uint32_t bhVKWorldPipeline::s_numInst{ 0 };

static const char* g_SamplerIdxNames[bhVKWorldPipeline::SamplerIdx::NUM_SAMPLERS] =
{
  "albedo",
  "normalspecular"
};

bool bhVKWorldPipeline::CreateLayout(const bhVKRenderDevice* rd)
{
  if (s_numInst > 0)
  {
    return true;
  }
  assert(rd);

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

  vkCreateDescriptorSetLayout(rd->Device(), &viewProjectionCI, rd->AllocCB(), &(bhVKWorldPipeline::s_descSetLayouts_v[bhVKWorldPipeline::DESC_SET_VIEW_PROJECTION]));

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
  vkCreateDescriptorSetLayout(rd->Device(), &texturesCI, rd->AllocCB(), &(bhVKWorldPipeline::s_descSetLayouts_v[bhVKWorldPipeline::DESC_SET_TEXTURE]));

  VkPipelineLayoutCreateInfo plCI =
  {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0,
    (uint32_t)bhVKWorldPipeline::NUM_DESC_SETS,
    bhVKWorldPipeline::s_descSetLayouts_v,
    (uint32_t)pushConstantRanges_v.size(),
    pushConstantRanges_v.data()
  };

  s_layout = VK_NULL_HANDLE;
  vkCreatePipelineLayout(rd->Device(), &plCI, rd->AllocCB(), &s_layout); // TODO: Error check
  if (s_layout == VK_NULL_HANDLE)
  {
    return false;
  }
  ++s_numInst;
  return true;
}

void bhVKWorldPipeline::DestroyLayout(const bhVKRenderDevice* rd)
{
  if (s_numInst > 1)
  {
    return;
  }

  assert(rd != nullptr);
  vkDestroyPipelineLayout(rd->Device(), bhVKWorldPipeline::s_layout, rd->AllocCB());
  for (auto dsl = 0; dsl < bhVKWorldPipeline::NUM_DESC_SETS; ++dsl)
  {
    vkDestroyDescriptorSetLayout(rd->Device(), bhVKWorldPipeline::s_descSetLayouts_v[dsl], rd->AllocCB());
  }
  --s_numInst;
}

bhVKWorldPipeline* bhVKWorldPipeline::Create(const bhVKRenderDevice* rd, uint32_t w, uint32_t h)
{
  assert(rd != nullptr);
  if (!CreateLayout(rd))
  {
    return nullptr;
  }

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
  s0.stage = bhVK::GetShaderStage(SHADER_STG_VERTEX);
  s0.module = rd->CreateShaderFromFile(shaderNames_v[0]);
  s0.pName = "main";

  VkPipelineShaderStageCreateInfo& s1 = shaderStageCI_v[1];
  s1.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  s1.stage = bhVK::GetShaderStage(SHADER_STG_FRAGMENT);
  s1.module = rd->CreateShaderFromFile(shaderNames_v[1]);
  s1.pName = "main";

  pipelineCI.stageCount = STAGE_COUNT;
  pipelineCI.pStages = shaderStageCI_v;

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
  assert(s_layout != VK_NULL_HANDLE);
  pipelineCI.layout = s_layout;

  ////////////////////////////////////////////////////////////////////////////////
  // Renderpass
  pipelineCI.renderPass = rd->PresentRenderPass(); // TODO: Must generalize in order to render to texture (for example)
  pipelineCI.subpass = 0;

  ////////////////////////////////////////////////////////////////////////////////
  // Base Pipeline
  pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCI.basePipelineIndex = 0;

  ////////////////////////////////////////////////////////////////////////////////
  bhVKWorldPipeline* newPl = new bhVKWorldPipeline();
  VkResult result = vkCreateGraphicsPipelines(rd->Device(), VK_NULL_HANDLE, 1, &pipelineCI, rd->AllocCB(), &(newPl->pipeline));

  ////////////////////////////////////////////////////////////////////////////////
  // Destroy creation objects
  for (auto sh = 0; sh < STAGE_COUNT; ++sh)
  {
    vkDestroyShaderModule(rd->Device(), shaderStageCI_v[sh].module, rd->AllocCB());
  }

  // Setup descriptor sets
  VkDescriptorSetAllocateInfo descriptorSetAI =
  {
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
    rd->DescriptorPool(),
    (uint32_t)bhVKWorldPipeline::NUM_DESC_SETS,
    bhVKWorldPipeline::s_descSetLayouts_v
  };
  if (vkAllocateDescriptorSets(rd->Device(), &descriptorSetAI, newPl->descSets_v) != VK_SUCCESS)
  {
    // TODO: Report error
    assert(false);
  }

  // Setup camera memory
  newPl->cameraVPBuffer = rd->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(bhCamera::ViewData));
  {
    VkDescriptorBufferInfo bufferInfo = { newPl->cameraVPBuffer.buffer, 0, VK_WHOLE_SIZE };
    VkWriteDescriptorSet writeDescSet =
    {
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
      newPl->descSets_v[bhVKWorldPipeline::DESC_SET_VIEW_PROJECTION],
      0,
      0,
      1,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      nullptr,
      &bufferInfo,
      nullptr
    };
    vkUpdateDescriptorSets(rd->Device(), 1, &writeDescSet, 0, nullptr);
  }

  VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  newPl->vb = rd->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, propertyFlags, sizeof(bhMeshVertex) * bhVKWorldPipeline::MAX_VERTS);
  newPl->ib = rd->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, propertyFlags, sizeof(bhMeshIdx_t) * bhVKWorldPipeline::MAX_INDS);

  return newPl;
}

void bhVKWorldPipeline::Destroy(const bhVKRenderDevice* rd, bhVKWorldPipeline*& pl)
{
  assert(rd != nullptr);

  rd->DestroyBuffer(pl->ib);
  rd->DestroyBuffer(pl->vb);
  rd->DestroyBuffer(pl->cameraVPBuffer);
  vkDestroyPipeline(rd->Device(), pl->pipeline, rd->AllocCB());
  delete pl;
  pl = nullptr;

  DestroyLayout(rd);
}

void bhVKWorldPipeline::Bind(const bhVKRenderDevice* rd, const bhVKWorldPipeline* pipeline)
{
  VkCommandBuffer currCB = rd->CurrCmdBuffer();
  vkCmdBindPipeline(rd->CurrCmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

  vkCmdBindDescriptorSets(
    currCB,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    s_layout,
    bhVKWorldPipeline::DESC_SET_VIEW_PROJECTION,
    1,
    &(pipeline->descSets_v[bhVKWorldPipeline::DESC_SET_VIEW_PROJECTION]),
    0,
    nullptr);

  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(currCB, 0, 1, &(pipeline->vb.buffer), offsets);
  vkCmdBindIndexBuffer(currCB, pipeline->ib.buffer, 0, BH_MESH_INDEX_TYPE);
}

void bhVKWorldPipeline::SetWorldCameraView(const bhVKRenderDevice* rd, const bhCamera::ViewData* vd) const
{
  rd->CopyDataToBuffer(cameraVPBuffer, 0, sizeof(bhCamera::ViewData), vd);
}

bool bhVKWorldPipeline::UploadWorldMesh(const bhVKRenderDevice* rd, bhMesh* mesh)
{
  if (mesh->deviceData.IsUploaded())
  {
    return false;
  }
  mesh->deviceData.offsets.vertexOffset = vertexOffset;
  vertexOffset += mesh->GetNumVerts();
  VkDeviceSize reqVertexSize = mesh->GetNumVerts() * sizeof(bhMeshVertex);
  rd->CopyDataToBuffer(vb, vbMemOffs, reqVertexSize, mesh->GetVerts());
  vbMemOffs += reqVertexSize;

  mesh->deviceData.offsets.indexOffset = indexOffset;
  indexOffset += mesh->GetNumInds();
  VkDeviceSize reqIdxSize = mesh->GetNumInds() * sizeof(bhMeshIdx_t);
  rd->CopyDataToBuffer(ib, ibMemOffs, reqIdxSize, mesh->GetInds());
  ibMemOffs += reqIdxSize;

  mesh->deviceData.SetUploaded();
  return true;
}

void bhVKWorldPipeline::DrawWorldMesh(const bhVKRenderDevice* rd, const bhMesh* mesh, const glm::mat4& transform) const
{
  VkCommandBuffer currCB = rd->CurrCmdBuffer();
  vkCmdPushConstants(currCB, s_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
  vkCmdDrawIndexed(currCB, mesh->GetNumInds(), 1, mesh->deviceData.offsets.indexOffset, mesh->deviceData.offsets.vertexOffset, 0);
}

bhVKWorldPipeline::Material* bhVKWorldPipeline::CreateMaterial(const bhVKRenderDevice* rd, const bhVKWorldPipeline::MaterialCreateInfo& mci)
{
  Material* newMat = new Material();

  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
  {
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
    rd->DescriptorPool(),
    1,
    &(s_descSetLayouts_v[bhVKWorldPipeline::DESC_SET_TEXTURE])
  };

  if (vkAllocateDescriptorSets(rd->Device(), &descriptorSetAllocateInfo, &(newMat->texturesDescriptorSet)) != VK_SUCCESS)
  {
    // TODO: Report error
    assert(false);
  }

  assert(newMat->texturesDescriptorSet != VK_NULL_HANDLE);
  bool texturesValid = true;
  {
    const bhVKTexture* tx = rd->GetTexture(mci.textureNames[bhVKWorldPipeline::SAMPLER_ALBEDO]);
    newMat->textureInputs_v[bhVKWorldPipeline::SAMPLER_ALBEDO] = rd->SetupTextureDescriptor(*tx);
    texturesValid &= bhVK::IsTextureInputValid(newMat->textureInputs_v[bhVKWorldPipeline::SAMPLER_ALBEDO]);
  }
  {
    const bhVKTexture* tx = rd->GetTexture(mci.textureNames[bhVKWorldPipeline::SAMPLER_NORMAL_SPECULAR]);
    newMat->textureInputs_v[bhVKWorldPipeline::SAMPLER_NORMAL_SPECULAR] = rd->SetupTextureDescriptor(*tx);
    texturesValid &= bhVK::IsTextureInputValid(newMat->textureInputs_v[bhVKWorldPipeline::SAMPLER_NORMAL_SPECULAR]);
  }
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
    bhVKWorldPipeline::NUM_SAMPLERS,
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    newMat->textureInputs_v
  };
  vkUpdateDescriptorSets(rd->Device(), 1, &writeDescSet, 0, nullptr);
  return newMat;
}

void bhVKWorldPipeline::DestroyMaterial(const bhVKRenderDevice* rd, const bhVKWorldPipeline::Material* mat)
{
  for (uint16_t s = 0; s < bhVKWorldPipeline::NUM_SAMPLERS; ++s)
  {
    vkDestroySampler(rd->Device(), mat->textureInputs_v[s].sampler, rd->AllocCB());
    vkDestroyImageView(rd->Device(), mat->textureInputs_v[s].imageView, rd->AllocCB());
  }
  // In order to allow individual release of descriptor sets
  // we must create the descriptor pool with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT flag
  //vkFreeDescriptorSets(device, descriptorPool, 1, &mat->textureDescriptorsSet);
}

void bhVKWorldPipeline::BindMaterial(const bhVKRenderDevice* rd, const bhVKWorldPipeline::Material* mat)
{
  vkCmdBindDescriptorSets(
    rd->CurrCmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, s_layout,
    bhVKWorldPipeline::DESC_SET_TEXTURE, 1, &(mat->texturesDescriptorSet), 0, nullptr);
}

bhVKWorldPipeline::Material* bhVKWorldPipeline::LoadMaterial(const bhVKRenderDevice* rd, const char* fileName)
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
    bhVKWorldPipeline::MaterialCreateInfo mci;
    for (int si = 0; si < bhVKWorldPipeline::SamplerIdx::NUM_SAMPLERS; ++si)
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
      return CreateMaterial(rd, mci);
    }
  }
  return nullptr;
}
