#ifndef BH_WORLD_PIPELINE_HPP
#define BH_WORLD_PIPELINE_HPP

#include "VK/bhStructsVK.hpp"

////////////////////////////////////////////////////////////////////////////////
struct bhPipelineVK_World;
//struct bhModelData;

struct bhWorldMaterial
{
    const bhPipelineVK_World* pipeline { nullptr };
    VkDescriptorImageInfo textureInputs_v[2];
    VkDescriptorSet textureDescriptorsSet = VK_NULL_HANDLE;
};

////////////////////////////////////////////////////////////////////////////////
struct bhPipelineVK_World
{
    enum BindingSet
    {
        BINDING_SET_VIEW,
        //BINDING_SET_MODEL,
        BINDING_SET_TEXTURES,

        NUM_BINDING_SETS
    };

    VkPipeline pipeline { VK_NULL_HANDLE };
    VkDescriptorSetLayout descSetLayouts_v[NUM_BINDING_SETS];
    VkDescriptorSet descSets_v[NUM_BINDING_SETS];
    VkPipelineLayout layout { VK_NULL_HANDLE };
    
    bhBufferVK vertexBuffer;
    VkDeviceSize vertexBufferWriteOffset{ 0 };
    bhBufferVK indexBuffer;
    VkDeviceSize indexBufferWriteOffset{ 0 };
    //bhBufferVK dynamicModelBuffer;
    bhBufferVK viewDataBuffer;

    //bhModelData* modelData { nullptr };
    //size_t dynamicAlignment { 0 };
};

#endif //BH_WORLD_PIPELINE_HPP
