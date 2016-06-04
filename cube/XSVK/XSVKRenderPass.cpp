//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKRenderPass.h"

using namespace std;
using namespace XSVK;

RenderPass::RenderPass(const VkDevice pVkDevice) :
	m_pVkDevice(pVkDevice),
	m_vkSampleCount(VK_SAMPLE_COUNT_1_BIT),
	m_vVkColorAtts(0)
{
	m_VkDepthAtt.format = VK_FORMAT_UNDEFINED;

	m_VkBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	m_VkBegin.pNext = nullptr;
	m_VkBegin.renderPass = VK_NULL_HANDLE;
}

RenderPass::~RenderPass()
{
	Reset();
}

void RenderPass::Create()
{
	const auto uNumColorAtt = uint8_t(m_vVkColorAtts.size());
	auto vAttachments = m_vVkColorAtts;
	if (m_VkDepthAtt.format)
		vAttachments.push_back(m_VkDepthAtt);

	vector<VkAttachmentReference> vColorRefs(uNumColorAtt);
	for (auto i = 0u; i < uNumColorAtt; ++i)
	{
		vColorRefs[i] =
		{
			i,															//.attachment = i, 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL					//.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
	}
	const VkAttachmentReference depthRef =
	{
		uNumColorAtt,													//.attachment = 1,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL				//.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	const VkSubpassDescription subpass =
	{
		0u,																//.flags = 0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,								//.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		0u,																//.inputAttachmentCount = 0,
		nullptr,														//.pInputAttachments = NULL,
		uNumColorAtt,													//.colorAttachmentCount = 1,
		vColorRefs.data(),												//.pColorAttachments = &color_reference,
		nullptr,														//.pResolveAttachments = NULL,
		m_VkDepthAtt.format ? &depthRef : nullptr,						//.pDepthStencilAttachment = &depth_reference,
		0u,																//.preserveAttachmentCount = 0,
		nullptr															//.pPreserveAttachments = NULL,
	};

	const VkRenderPassCreateInfo renderpassInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,						//.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,														//.pNext = NULL,
		0u,																//.flags = 0,
		uint32_t(vAttachments.size()),									//.attachmentCount = 2,
		vAttachments.data(),											//.pAttachments = attachments,
		1u,																//.subpassCount = 1,
		&subpass,														//.pSubpasses = &subpass,
		0u,																//.dependencyCount = 0,
		nullptr,														//.pDependencies = NULL,
	};

	VkResult U_ASSERT_ONLY err = vkCreateRenderPass(m_pVkDevice,
		&renderpassInfo, nullptr, &m_VkBegin.renderPass);
	assert(!err);
}

void RenderPass::SetNumColorAtts(uint8_t uNum)
{
	m_vVkColorAtts.resize(uNum);
	m_vVkColorAtts.shrink_to_fit();
	assert(m_vVkColorAtts.data());
}

void RenderPass::SetColorBuffer(uint8_t i, const VkFormat vkFormat, const VkAttachmentLoadOp vkLoadOp)
{
	m_vVkColorAtts[i] =
	{
		0u,															//.flags = 0,
		vkFormat,													//.format = demo->format,
		m_vkSampleCount,											//.samples = VK_SAMPLE_COUNT_1_BIT,
		vkLoadOp,													//.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,								//.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,							//.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,							//.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,					//.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL					//.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
}

void RenderPass::SetColorBuffer(uint8_t i, const upImage &pColorBuffer, const VkAttachmentLoadOp vkLoadOp)
{
	SetColorBuffer(i, pColorBuffer->GetFormat(), vkLoadOp);
}

void RenderPass::SetDepthBuffer(const upImage &pDepthBuffer, const VkAttachmentLoadOp vkLoadOp)
{
	m_VkDepthAtt =
	{
		0u,													//.flags = 0,
		pDepthBuffer->GetFormat(),							//.format = demo->depth.format,
		m_vkSampleCount,									//.samples = VK_SAMPLE_COUNT_1_BIT,
		vkLoadOp,											//.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,										
		VK_ATTACHMENT_STORE_OP_DONT_CARE,					//.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,					//.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,					//.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,	//.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL	//.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
}

void RenderPass::AttachColorBuffer(const VkFormat vkFormat, const VkAttachmentLoadOp vkLoadOp)
{
	const auto attachment = VkAttachmentDescription
	{
		0u,															//.flags = 0,
		vkFormat,													//.format = demo->format,
		m_vkSampleCount,											//.samples = VK_SAMPLE_COUNT_1_BIT,
		vkLoadOp,													//.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,								//.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,							//.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,							//.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,					//.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL					//.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	m_vVkColorAtts.push_back(attachment);
	assert(m_vVkColorAtts.data());
}

void RenderPass::AttachColorBuffer(const upImage &pColorBuffer, const VkAttachmentLoadOp vkLoadOp)
{
	AttachColorBuffer(pColorBuffer->GetFormat(), vkLoadOp);
}

void RenderPass::SetFramebuffer(const VkFramebuffer vkFramebuffer)
{
	m_VkBegin.framebuffer = vkFramebuffer;
}

void RenderPass::SetRenderArea(
	const uint32_t uWidth, const uint32_t uHeight,
	const int iOffsetX, const int iOffsetY)
{
	m_VkBegin.renderArea.extent.width = uWidth;
	m_VkBegin.renderArea.extent.height = uHeight;
	m_VkBegin.renderArea.offset.x = iOffsetX;
	m_VkBegin.renderArea.offset.y = iOffsetY;
}

void RenderPass::SetClearColors(const pVkClearValue pVkClears, const uint8_t uNum)
{
	m_VkBegin.pClearValues = pVkClears;
	m_VkBegin.clearValueCount = uNum;
}

void RenderPass::SetSampleCount(const VkSampleCountFlagBits vkSampleCount)
{
	m_vkSampleCount = vkSampleCount;
}

void RenderPass::Reset()
{
	if (m_VkBegin.renderPass)
		vkDestroyRenderPass(m_pVkDevice, m_VkBegin.renderPass, nullptr);

	m_vVkColorAtts.clear();
}

const VkRenderPass &RenderPass::Get() const
{
	return m_VkBegin.renderPass;
}

const VkRenderPassBeginInfo &RenderPass::GetBegin() const
{
	return m_VkBegin;
}
