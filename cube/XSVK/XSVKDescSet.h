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
		void Reset();

		void SetNumBindings(const uint8_t uSet, const uint8_t uNum);
		void AttachBindings(const uint8_t uSet, const uint8_t uNum,
			const VkDescriptorType vkType, const VkShaderStageFlags vkStage);
		void SetBindings(const uint8_t uSet, const uint8_t uBinding,
			const uint8_t uNum, const VkDescriptorType vkType,
			const VkShaderStageFlags vkStage);
		void SetBuffers(const uint8_t uSet, const uint8_t uBinding,
			const ppBuffer ppBuffers);
		void SetBuffers(const uint8_t uSet, const uint8_t uStart,
			const uint8_t uCount, const ppBuffer ppBuffers);
		void SetTextures(const uint8_t uSet, const uint8_t uBinding,
			const ppTexture ppTextures);
		void SetTextures(const uint8_t uSet, const uint8_t uBinding,
			const ppImage ppTextures);
		void SetTextures(const uint8_t uSet, const uint8_t uStart,
			const uint8_t uCount, const ppTexture ppTextures);
		void SetTextures(const uint8_t uSet, const uint8_t uStart,
			const uint8_t uCount, const ppImage ppTextures);

		const pVkDescriptorSet			Get() const;
		const vVkDescriptorSetLayout	&GetLayouts() const;
		const VkDescriptorSet			&Get(const uint8_t i) const;
		const VkDescriptorSetLayout		&GetLayout(const uint8_t i) const;
	protected:
		using vVkDescriptorSet = std::vector<VkDescriptorSet>;
		using vVkDescriptorSetLayoutBinding = std::vector<VkDescriptorSetLayoutBinding>;
		using vvVkDescriptorSetLayoutBinding = std::vector<vVkDescriptorSetLayoutBinding>;
		//using vVkDescriptorBufferInfo = std::vector<VkDescriptorBufferInfo>;
		//using vVkDescriptorImageInfo = std::vector<VkDescriptorImageInfo>;

		void setBindings(const uint8_t s, const uint8_t i, const uint8_t uNum,
			const VkShaderStageFlags vkStage, const VkDescriptorType vkType);

		VkDescriptorPool				m_VkDescPool;

		vVkDescriptorSetLayout			m_vVkDescLayouts;
		vVkDescriptorSet				m_vVkDescSets;

		vvVkDescriptorSetLayoutBinding	m_vvVkBindings;

		const VkDevice	m_pVkDevice;
	};
}
