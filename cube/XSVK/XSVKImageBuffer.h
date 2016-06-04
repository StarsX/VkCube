//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include "XSVKBuffer.h"

namespace XSVK
{
	class ImageBuffer :
		public Buffer
	{
	public:
		ImageBuffer(const VkDevice pVkDevice,
			const VkPhysicalDeviceMemoryProperties &vkMemProps);
		virtual ~ImageBuffer();

		void CreateDepth();
		void Create(const VkImage vkImage = VK_NULL_HANDLE,
			const bool bSampler = true,
			const VkFlags uReqProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		void CreateSampler();
		void AllocCommandBuffer(const VkCommandPool &vkCmdPool);
		void SetFormat(const VkFormat vkFormat);
		void SetDimensions(const uint32_t uWidth, const uint32_t uHeight);
		void SetUsage(const VkImageUsageFlags uUsage,
			const VkImageTiling vkTiling = VK_IMAGE_TILING_OPTIMAL);
		void SetFilter(const VkFilter vkMin, const VkFilter vkMag,
			const VkSamplerMipmapMode vkMip);
		void SetAddressMode(const VkSamplerAddressMode vkAddrMode,
			const VkBorderColor vkColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);
		void SetMipLevels(const uint8_t uMips);
		void SetSampleCount(const VkSampleCountFlagBits vkSampleCount);
		void SetAnisotropy(const VkBool32 vkEnable,
			const float fMaxAniso);
		void SetBarrier(const VkImageLayout vkImgLayout,
			const VkCommandBuffer &pVkCmd = nullptr,
			VkPipelineStageFlags vkDstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			const uint8_t uMip = 0ui8);
		void GenerateMips(const VkImageLayout vkImgLayout, const VkCommandBuffer &pVkCmd,
			const VkPipelineStageFlags vkDstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		void Reset();

		const VkImage				&GetImage() const;
		const VkDescriptorImageInfo	&GetDesc() const;
		const VkFormat				&GetFormat() const;
		const VkCommandBuffer		&GetCmd() const;

	protected:
		using vVkImageView = std::vector<VkImageView>;

		void createImage(const VkImage pVkImage,
			const VkFlags uReqProps);
		void createView();

		VkImage						m_VkImage;
		VkDescriptorImageInfo		m_VkDesc;
		VkImageCreateInfo			m_VkImageInfo;
		VkSamplerCreateInfo			m_VkSampler;

		VkCommandBuffer				m_pVkCmd;
		VkCommandPool				m_VkCmdPool;
	};
}
