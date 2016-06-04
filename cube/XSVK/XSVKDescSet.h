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
	class DescSet
	{
	public:
		using upBuffer = const std::unique_ptr<Buffer>;
		using ppBuffer = std::add_pointer_t<upBuffer>;
		using upImage = const std::unique_ptr<ImageBuffer>;
		using ppImage = std::add_pointer_t<upImage>;
		using upTexture = const std::unique_ptr<Texture>;
		using ppTexture = std::add_pointer_t<upTexture>;

		DescSet(const VkDevice pVkDevice);
		virtual ~DescSet();

		void Create();
		void CreateDescLayout();
		void CreateDescPool();
		void CreateDescSet();
		void Reset();

		void SetNumBindings(const uint8_t uNum);
		void SetBuffers(const uint8_t i, const uint8_t uNum,
			const ppBuffer ppBuffers,
			const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		void AttachBuffers(const uint8_t uNum,
			const ppBuffer ppBuffers,
			const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		void SetTextures(const uint8_t i, const uint8_t uNum,
			const ppTexture ppTextures,
			const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		void SetTextures(const uint8_t i, const uint8_t uNum,
			const ppImage ppTextures,
			const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		void AttachTextures(const uint8_t uNum,
			const ppTexture ppTextures,
			const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		void AttachTextures(const uint8_t uNum,
			const ppImage ppTextures,
			const VkShaderStageFlags vkStage,
			const VkDescriptorType vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		const VkDescriptorSet			&Get() const;
		const VkDescriptorSetLayout		&GetLayout() const;
	protected:
		using vVkDescriptorSetLayoutBinding = std::vector<VkDescriptorSetLayoutBinding>;
		using vVkWriteDescriptorSet = std::vector<VkWriteDescriptorSet>;
		using vVkDescriptorBufferInfo = std::vector<VkDescriptorBufferInfo>;
		using vvVkDescriptorBufferInfo = std::vector<vVkDescriptorBufferInfo>;
		using vVkDescriptorImageInfo = std::vector<VkDescriptorImageInfo>;
		using vvVkDescriptorImageInfo = std::vector<vVkDescriptorImageInfo>;

		VkDescriptorSetLayout			m_VkDescLayout;
		VkDescriptorPool				m_VkDescPool;
		VkDescriptorSet					m_VkDescSet;

		vVkDescriptorSetLayoutBinding	m_vVkBindings;
		vVkWriteDescriptorSet			m_vVkWrites;

		vvVkDescriptorBufferInfo		m_vvBufferDesc;
		vvVkDescriptorImageInfo			m_vvImageDesc;

		const VkDevice	m_pVkDevice;
	};
}
