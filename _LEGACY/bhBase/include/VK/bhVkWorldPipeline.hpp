#ifndef BH_WORLD_PIPELINE_HPP
#define BH_WORLD_PIPELINE_HPP

#include "VK/bhVk.hpp"

namespace bhVk
{
	struct WorldPipeline
	{
		static constexpr size_t MAX_VERTS = 1024 << 1;
		static constexpr size_t MAX_TRIS = 1024 << 1;
		static constexpr size_t MAX_INDS = 3 * MAX_TRIS;

		enum DescSetIdx
		{
			DESC_SET_VIEW_PROJECTION,
			DESC_SET_TEXTURE,

			NUM_DESC_SETS
		};

		enum SamplerIdx
		{
			SAMPLER_ALBEDO,
			SAMPLER_NORMAL_SPECULAR,

			NUM_SAMPLERS
		};

		struct Material
		{
			VkDescriptorImageInfo textureInputs_v[NUM_SAMPLERS] = {};
			VkDescriptorSet texturesDescriptorSet{ VK_NULL_HANDLE };
		};

		struct MaterialCreateInfo
		{
			const char* textureNames[NUM_SAMPLERS] = {};
		};

		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout layout{ VK_NULL_HANDLE };
		VkDescriptorSetLayout descSetLayouts_v[NUM_DESC_SETS] = {};
		VkDescriptorSet descSets_v[NUM_DESC_SETS] = {};
		Buffer cameraVPBuffer; // TODO: Should keep mapped

		Buffer vb;
		VkDeviceSize vbMemOffs{ 0 };
		int32_t vertexOffset{ 0 };

		Buffer ib;
		VkDeviceSize ibMemOffs{ 0 };
		uint32_t indexOffset{ 0 };
	};
}

#endif //BH_WORLD_PIPELINE_HPP
