//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKFramebuffer.h"

using namespace std;
using namespace XSVK;

Framebuffer::Framebuffer(const VkDevice pVkDevice) :
	m_pVkDevice(pVkDevice),
	m_VkFramebuffer(VK_NULL_HANDLE),
	m_VkDepthAtt(VK_NULL_HANDLE),
	m_vVkColorAtts(0)
{
	memset(&m_VkFbInfo, 0, sizeof(VkFramebufferCreateInfo));
	m_VkFbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_VkFbInfo.layers = 1u;
}

Framebuffer::~Framebuffer()
{
	Reset();
}

void Framebuffer::Create()
{
	auto vAttachments = m_vVkColorAtts;
	if (m_VkDepthAtt) vAttachments.push_back(m_VkDepthAtt);

	m_VkFbInfo.attachmentCount = uint32_t(vAttachments.size());
	m_VkFbInfo.pAttachments = vAttachments.data();

	VkResult U_ASSERT_ONLY err = vkCreateFramebuffer(
		m_pVkDevice, &m_VkFbInfo, nullptr, &m_VkFramebuffer);
	assert(!err);
}

void Framebuffer::SetDimensions(const uint32_t uWidth, const uint32_t uHeight)
{
	m_VkFbInfo.width = uWidth;
	m_VkFbInfo.height = uHeight;
}

void Framebuffer::SetRenderPass(const VkRenderPass &vkRenderPass)
{
	m_VkFbInfo.renderPass = vkRenderPass;
}

void Framebuffer::SetNumColorBuffers(const uint8_t uNum)
{
	m_vVkColorAtts.resize(uNum);
	m_vVkColorAtts.shrink_to_fit();
}

void Framebuffer::SetColorBuffers(const ppImage ppColorBuffers,
	const uint8_t uNum)
{
	m_vVkColorAtts.resize(uNum);
	m_vVkColorAtts.shrink_to_fit();
	assert(m_vVkColorAtts.data());
	for (auto i = 0ui8; i < uNum; ++i)
		m_vVkColorAtts[i] = ppColorBuffers[i]->GetDesc().imageView;
}

void Framebuffer::SetColorBuffer(const uint8_t i, const upImage &pColorBuffer)
{
	m_vVkColorAtts[i] = pColorBuffer->GetDesc().imageView;
}

void Framebuffer::SetDepthBuffer(const upImage &pDepthBuffer)
{
	m_VkDepthAtt = pDepthBuffer->GetDesc().imageView;
}

void Framebuffer::AttachColorBuffer(const upImage &pColorBuffer)
{
	m_vVkColorAtts.push_back(pColorBuffer->GetDesc().imageView);
	assert(m_vVkColorAtts.data());
}

void Framebuffer::Reset()
{
	if (m_VkFramebuffer)
		vkDestroyFramebuffer(m_pVkDevice, m_VkFramebuffer, nullptr);
}

const VkFramebuffer &Framebuffer::Get() const
{
	return m_VkFramebuffer;
}
