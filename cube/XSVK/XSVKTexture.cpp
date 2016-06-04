//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKTexture.h"

using namespace std;
using namespace XSVK;

Texture::Texture(const VkDevice pVkDevice,
	const VkPhysicalDeviceMemoryProperties &vkMemProps) :
	ImageBuffer(pVkDevice, vkMemProps)
{
}

void Texture::Create(const VkCommandPool &vkCmdPool, const bool bStaging,
	const bool bSampler)
{
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(m_pVkGPU, m_VkImageInfo.format, &props);

	initCmd(vkCmdPool);

	if ((props.linearTilingFeatures &
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
		!bStaging)
	{
		/* Device can texture using linear textures */
		SetUsage(VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
		CreateImage(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}
	else if (props.optimalTilingFeatures &
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
	{
		/* Must use staging buffer to copy linear texture to optimized */
		Texture stagingTex(m_pVkDevice, m_VkMemProps);

		stagingTex.SetFormat(m_VkImageInfo.format);
		stagingTex.SetDimensions(
			m_VkImageInfo.extent.width,
			m_VkImageInfo.extent.height);
		stagingTex.SetUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_IMAGE_TILING_LINEAR);
		stagingTex.SetLoadFunc(m_FxLoadData);
		stagingTex.CreateImage(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		SetUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_TILING_LINEAR);
		CreateImage(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		stagingTex.SetBarrier(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_pVkCmd);

		VkImageCopy copyRegion =
		{
			{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u },	//.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			{ 0, 0, 0 },								//.srcOffset = {0, 0, 0},
			{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u },	//.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
			{ 0, 0, 0 },								//.dstOffset = {0, 0, 0},
			m_VkImageInfo.extent,						//.extent = {staging_texture.tex_width, staging_texture.tex_height, 1},
		};
		vkCmdCopyImage(m_pVkCmd, stagingTex.GetImage(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_VkImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copyRegion);
	}
	else
		// Can't support VK_FORMAT_R8G8B8A8_UNORM !?
		assert(!"No support for R8G8B8A8_UNORM as texture image format");

	// setting the image layout does not reference the actual memory so no need to add a mem ref
	SetBarrier(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	flushInitCmd();

	// create image view
	createView();

	// create sampler
	if (bSampler) CreateSampler();
}

void Texture::CreateImage(const VkFlags uReqProps)
{
	if (uReqProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		m_VkImageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		m_VkDesc.imageLayout = m_VkImageInfo.initialLayout;
	}
	else
	{
		m_VkImageInfo.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		m_VkDesc.imageLayout = m_VkImageInfo.initialLayout;
	}

	createImage(VK_NULL_HANDLE, uReqProps);

	if (uReqProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		const VkImageSubresource subres =
		{
			VK_IMAGE_ASPECT_COLOR_BIT,	//.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			0u,							//.mipLevel = 0,
			0u							//.arrayLayer = 0,
		};
		VkSubresourceLayout layout;
		lpvoid pData;

		vkGetImageSubresourceLayout(m_pVkDevice, m_VkImage, &subres, &layout);
		Map(&pData);
		m_FxLoadData(pData, layout);
		Unmap();
	}
}

void Texture::SetGPUQueue(const VkPhysicalDevice pVkGPU, const VkQueue pVkQueue)
{
	m_pVkGPU = pVkGPU;
	m_pVkQueue = pVkQueue;
}

void Texture::SetLoadFunc(const FxLoadTexture &fxLoadData)
{
	m_FxLoadData = fxLoadData;
}

void Texture::initCmd(const VkCommandPool &vkCmdPool)
{
	if (!m_pVkCmd) AllocCommandBuffer(vkCmdPool);

	static const auto cmdBufHInfo = VkCommandBufferInheritanceInfo
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,	//.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		nullptr,											//.pNext = NULL,
		VK_NULL_HANDLE,										//.renderPass = VK_NULL_HANDLE,
		0u,													//.subpass = 0,
		VK_NULL_HANDLE,										//.framebuffer = VK_NULL_HANDLE,
		VK_FALSE,											//.occlusionQueryEnable = VK_FALSE,
		0x00000000,											//.queryFlags = 0,
		0x00000000											//.pipelineStatistics = 0,
	};
	static const VkCommandBufferBeginInfo cmdBufInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,		//.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,											//.pNext = NULL,
		0x00000000,											//.flags = 0,
		&cmdBufHInfo										//.pInheritanceInfo = &cmd_buf_hinfo,
	};
	VkResult U_ASSERT_ONLY err = vkBeginCommandBuffer(m_pVkCmd, &cmdBufInfo);
	assert(!err);
}

void Texture::flushInitCmd()
{
	if (!m_pVkCmd) return;

	VkResult U_ASSERT_ONLY err = vkEndCommandBuffer(m_pVkCmd);
	assert(!err);

	VkFence nullFence = VK_NULL_HANDLE;
	const VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,	//.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,						//.pNext = NULL,
		0u,								//.waitSemaphoreCount = 0,
		nullptr,						//.pWaitSemaphores = NULL,
		nullptr,						//.pWaitDstStageMask = NULL,
		1u,								//.commandBufferCount = 1,
		&m_pVkCmd,						//.pCommandBuffers = cmd_bufs,
		0u,								//.signalSemaphoreCount = 0,
		nullptr							//.pSignalSemaphores = NULL
	};

	err = vkQueueSubmit(m_pVkQueue, 1u, &submitInfo, nullFence);
	assert(!err);

	err = vkQueueWaitIdle(m_pVkQueue);
	assert(!err);

	vkFreeCommandBuffers(m_pVkDevice, m_VkCmdPool, 1u, &m_pVkCmd);
	m_pVkCmd = nullptr;
}
