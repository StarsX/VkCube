//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <cassert>
#include <memory>
#include <vulkan/vulkan.h>
#include "XSVKImageBuffer.h"

namespace XSVK
{
	class Framebuffer
	{
	public:
		using upImage = std::unique_ptr<ImageBuffer>;
		using ppImage = std::add_pointer_t<const upImage>;

		Framebuffer(const VkDevice pVkDevice);
		virtual ~Framebuffer();

		void Create();
		void SetDimensions(const uint32_t uWidth, const uint32_t uHeight);
		void SetRenderPass(const VkRenderPass &vkRenderPass);
		void SetNumColorBuffers(const uint8_t uNum);
		void SetColorBuffers(const ppImage ppColorBuffers, const uint8_t uNum);
		void SetColorBuffer(const uint8_t i, const upImage &pColorBuffer);
		void SetDepthBuffer(const upImage &pDepthBuffer);
		void AttachColorBuffer(const upImage &pColorBuffer);
		void Reset();

		const VkFramebuffer		&Get() const;

	protected:
		using vVkImageView = std::vector<VkImageView>;

		VkFramebuffer			m_VkFramebuffer;
		VkFramebufferCreateInfo	m_VkFbInfo;
		vVkImageView			m_vVkColorAtts;
		VkImageView				m_VkDepthAtt;

		const VkDevice	m_pVkDevice;
	};
}
