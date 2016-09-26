//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <cassert>
#include <memory>
#include <vulkan/vulkan.h>
#include "XSVKTexture.h"

namespace XSVK
{
	using pVkDescriptorSet = std::add_pointer_t<const VkDescriptorSet>;
	using pVkDescriptorSetLayout = std::add_pointer_t<const VkDescriptorSetLayout>;
	using vVkDescriptorSetLayout = std::vector<VkDescriptorSetLayout>;

	class DescSet
	{
	public:
		using upBuffer = const std::unique_ptr<Buffer>;
		using ppBuffer = std::add_pointer_t<upBuffer>;
		using upImage = const std::unique_ptr<ImageBuffer>;
		using ppImage = std::add_pointer_t<upImage>;
		using upTexture = const std::unique_ptr<Texture>;
		using ppTexture = std::add_pointer_t<upTexture>;

		DescSet(const VkDevice pVkDevice, const uint8_t uNumSet = 0ui8);
		virtual ~DescSet();

		void Create();
		void CreateDescPool();
		void CreateDescLayouts();
		void CreateDescSets();
		void Update();
		void Reset();

		void SetNumBindings(const uint8_t s, const uint8_t uNum);
		void SetBuffers(const uint8_t s, const uint8_t i, const uint8_t uNum,
			const ppBuffer ppBuffers, const VkShaderStageFlags vkStage = 0,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		void AttachBuffers(const uint8_t s, const uint8_t uNum,
			const ppBuffer ppBuffers, const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		void SetTextures(const uint8_t s, const uint8_t i, const uint8_t uNum,
			const ppTexture ppTextures, const VkShaderStageFlags vkStage = 0,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		void SetTextures(const uint8_t s, const uint8_t i, const uint8_t uNum,
			const ppImage ppTextures, const VkShaderStageFlags vkStage = 0,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		void AttachTextures(const uint8_t s, const uint8_t uNum,
			const ppTexture ppTextures, const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		void AttachTextures(const uint8_t s, const uint8_t uNum,
			const ppImage ppTextures, const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		const pVkDescriptorSet			Get() const;
		const vVkDescriptorSetLayout	&GetLayouts() const;
		const VkDescriptorSet			&Get(const uint8_t i) const;
		const VkDescriptorSetLayout		&GetLayout(const uint8_t i) const;
	protected:
		using vVkDescriptorSet = std::vector<VkDescriptorSet>;
		using vVkDescriptorSetLayoutBinding = std::vector<VkDescriptorSetLayoutBinding>;
		using vvVkDescriptorSetLayoutBinding = std::vector<vVkDescriptorSetLayoutBinding>;
		using vVkWriteDescriptorSet = std::vector<VkWriteDescriptorSet>;
		using vVkDescriptorBufferInfo = std::vector<VkDescriptorBufferInfo>;
		using vvVkDescriptorBufferInfo = std::vector<vVkDescriptorBufferInfo>;
		using vVkDescriptorImageInfo = std::vector<VkDescriptorImageInfo>;
		using vvVkDescriptorImageInfo = std::vector<vVkDescriptorImageInfo>;

		void setBindings(const uint8_t s, const uint8_t i, const uint8_t uNum,
			const VkShaderStageFlags vkStage, const VkDescriptorType vkType);

		VkDescriptorPool				m_VkDescPool;

		vVkDescriptorSetLayout			m_vVkDescLayouts;
		vVkDescriptorSet				m_vVkDescSets;

		vvVkDescriptorSetLayoutBinding	m_vvVkBindings;
		vVkWriteDescriptorSet			m_vVkWrites;

		vvVkDescriptorBufferInfo		m_vvBufferDesc;
		vvVkDescriptorImageInfo			m_vvImageDesc;

		const VkDevice	m_pVkDevice;
	};
}
