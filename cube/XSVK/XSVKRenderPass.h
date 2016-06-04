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
	class RenderPass
	{
	public:
		using upImage = std::unique_ptr<ImageBuffer>;
		using pVkClearValue = std::add_pointer_t<const VkClearValue>;

		RenderPass(const VkDevice pVkDevice);
		virtual ~RenderPass();

		void Create();
		void SetNumColorAtts(uint8_t uNum);
		void SetColorBuffer(uint8_t i, const VkFormat vkFormat,
			const VkAttachmentLoadOp vkLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
		void SetColorBuffer(uint8_t i, const upImage &pColorBuffer,
			const VkAttachmentLoadOp vkLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
		void SetDepthBuffer(const upImage &pDepthBuffer,
			const VkAttachmentLoadOp vkLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
		void AttachColorBuffer(const VkFormat vkFormat,
			const VkAttachmentLoadOp vkLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
		void AttachColorBuffer(const upImage &pColorBuffer,
			const VkAttachmentLoadOp vkLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);

		void SetFramebuffer(const VkFramebuffer vkFramebuffer);
		void SetRenderArea(const uint32_t uWidth, const uint32_t uHeight,
			const int iOffsetX = 0, const int iOffsetY = 0);
		void SetClearColors(const pVkClearValue pVkClears, const uint8_t uNum);
		void SetSampleCount(const VkSampleCountFlagBits vkSampleCount);
		void Reset();

		const VkRenderPass			&Get() const;
		const VkRenderPassBeginInfo	&GetBegin() const;
	protected:
		using vVkAttachmentDescription = std::vector<VkAttachmentDescription>;

		VkRenderPassBeginInfo		m_VkBegin;
		vVkAttachmentDescription	m_vVkColorAtts;
		VkAttachmentDescription		m_VkDepthAtt;
		VkSampleCountFlagBits		m_vkSampleCount;

		const VkDevice				m_pVkDevice;
	};
}
