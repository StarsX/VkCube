//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKImageBuffer.h"

using namespace XSVK;

ImageBuffer::ImageBuffer(const VkDevice pVkDevice,
	const VkPhysicalDeviceMemoryProperties &vkMemProps) :
	Buffer(pVkDevice, vkMemProps),
	m_VkImage(VK_NULL_HANDLE),
	m_VkCmdPool(VK_NULL_HANDLE),
	m_pVkCmd(nullptr)
{
	memset(&m_VkDesc, 0, sizeof(VkDescriptorImageInfo));
	m_VkDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	memset(&m_VkImageInfo, 0, sizeof(VkImageCreateInfo));
	m_VkImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	m_VkImageInfo.imageType = VK_IMAGE_TYPE_2D;
	m_VkImageInfo.extent.depth = 1u;
	m_VkImageInfo.mipLevels = 1u;
	m_VkImageInfo.arrayLayers = 1u;
	m_VkImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	m_VkImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	m_VkImageInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	memset(&m_VkSampler, 0, sizeof(m_VkSampler));
	m_VkSampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	m_VkSampler.maxAnisotropy = 1.0f;
	m_VkSampler.maxLod = FLT_MAX;
}

ImageBuffer::~ImageBuffer()
{
	Reset();
}

void ImageBuffer::CreateDepth()
{
	SetUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	Create();
}

void ImageBuffer::Create(const VkImage pVkImage, const bool bSampler,
	const VkFlags uReqProps)
{
	// create image
	createImage(pVkImage, uReqProps);

	// create image view
	createView();

	// create sampler
	if (bSampler) CreateSampler();
}

void ImageBuffer::CreateSampler()
{
	if (m_VkImageInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		VkResult U_ASSERT_ONLY err = vkCreateSampler(m_pVkDevice,
			&m_VkSampler, nullptr, &m_VkDesc.sampler);
		assert(!err);
	}
}

void ImageBuffer::AllocCommandBuffer(const VkCommandPool &vkCmdPool)
{
	m_VkCmdPool = vkCmdPool;
	
	const VkCommandBufferAllocateInfo cmd =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	//.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,										//.pNext = NULL,
		m_VkCmdPool,									//.commandPool = demo->cmd_pool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,				//.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1u												//.commandBufferCount = 1,
	};

	VkResult U_ASSERT_ONLY err = vkAllocateCommandBuffers(m_pVkDevice, &cmd, &m_pVkCmd);
	assert(!err);
}

void ImageBuffer::SetFormat(const VkFormat vkFormat)
{
	m_VkImageInfo.format = vkFormat;
}

void ImageBuffer::SetDimensions(
	const uint32_t uWidth, const uint32_t uHeight)
{
	m_VkImageInfo.extent.width = uWidth;
	m_VkImageInfo.extent.height = uHeight;
}

void ImageBuffer::SetUsage(const VkImageUsageFlags uUsage,
	const VkImageTiling vkTiling)
{
	m_VkImageInfo.usage = uUsage;
	m_VkImageInfo.tiling = vkTiling;
}

void ImageBuffer::SetFilter(const VkFilter vkMin,
	const VkFilter vkMag, const VkSamplerMipmapMode vkMip)
{
	m_VkSampler.minFilter = vkMin;
	m_VkSampler.magFilter = vkMag;
	m_VkSampler.mipmapMode = vkMip;
}

void ImageBuffer::SetAddressMode(
	const VkSamplerAddressMode vkAddrMode,
	const VkBorderColor vkColor)
{
	m_VkSampler.addressModeU = vkAddrMode;
	m_VkSampler.addressModeV = vkAddrMode;
	m_VkSampler.addressModeW = vkAddrMode;
	m_VkSampler.borderColor = vkColor;
}

void ImageBuffer::SetMipLevels(const uint8_t uMips)
{
	const auto uMaxDim = max(m_VkImageInfo.extent.width, m_VkImageInfo.extent.height);
	const auto uMipMax = ilogbf(static_cast<float>(uMaxDim)) + 1u;
	m_VkImageInfo.mipLevels = uMips == 0 ? uMipMax : min(uMips, uMipMax);
	if (m_VkImageInfo.mipLevels > 1ui8)
		m_VkImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT;
}

void ImageBuffer::SetSampleCount(const VkSampleCountFlagBits vkSampleCount)
{
	m_VkImageInfo.samples = vkSampleCount;
}

void ImageBuffer::SetAnisotropy(const VkBool32 vkEnable, const float fMaxAniso)
{
	m_VkSampler.anisotropyEnable = vkEnable;
	m_VkSampler.maxAnisotropy = fMaxAniso;
}

void ImageBuffer::SetBarrier(const VkImageLayout vkImgLayout, const VkCommandBuffer &pVkCmd,
	VkPipelineStageFlags vkDstStages, const uint8_t uMip)
{
	auto barrier = VkImageMemoryBarrier
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,			//.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		nullptr,										//.pNext = NULL,
		0x00000000,										//.srcAccessMask = srcAccessMask,
		0x00000000,										//.dstAccessMask = 0,
		m_VkDesc.imageLayout,							//.oldLayout = old_image_layout,
		vkImgLayout,									//.newLayout = new_image_layout,
		VK_QUEUE_FAMILY_IGNORED,						//.srcQueueFamilyIndex = 0,
		VK_QUEUE_FAMILY_IGNORED,						//.dstQueueFamilyIndex = 0,
		m_VkImage,										//.image = image,
		{ VK_IMAGE_ASPECT_COLOR_BIT, uMip, 1u, 0u, 1u }	//.subresourceRange = {aspectMask, 0, 1, 0, 1}
	};

	m_VkDesc.imageLayout = vkImgLayout;

	VkPipelineStageFlags vkSrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	// Old layout analysis
	if (barrier.oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkSrcStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		vkSrcStages = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	// New layout analysis
	if (barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		/* Make sure anything that was copying from this image has completed */
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	else if (barrier.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	else if (barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		/* Make sure any Copy or CPU writes to image are flushed */
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	else if (barrier.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		vkSrcStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		vkDstStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}

	vkCmdPipelineBarrier(pVkCmd ? pVkCmd : m_pVkCmd, vkSrcStages, vkDstStages,
		0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
}

void ImageBuffer::GenerateMips(const VkImageLayout vkImgLayout,
	const VkCommandBuffer &pVkCmd, const VkPipelineStageFlags vkDstStages)
{
	auto vkImageBlit = VkImageBlit{ 0 };
	vkImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageBlit.srcSubresource.layerCount = 1u;
	vkImageBlit.dstSubresource.layerCount = 1u;

	for (auto i = 1ui8; i < m_VkImageInfo.mipLevels; ++i)
	{
		SetBarrier(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pVkCmd,
			vkDstStages, i - 1ui8);
		SetBarrier(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pVkCmd,
			vkDstStages, i);

		const auto iMipWidth = int32_t(m_VkImageInfo.extent.width >> i);
		const auto iMipHeight = int32_t(m_VkImageInfo.extent.height >> i);

		vkImageBlit.srcSubresource.mipLevel = i - 1u;
		vkImageBlit.srcOffsets[1] = { iMipWidth << 1, iMipHeight << 1, 1 };
		vkImageBlit.dstSubresource.mipLevel = i;
		vkImageBlit.dstOffsets[1] = { iMipWidth, iMipHeight, 1 };

		vkCmdBlitImage(pVkCmd, m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1u, &vkImageBlit, VK_FILTER_LINEAR);

		SetBarrier(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pVkCmd,
			vkDstStages, i - 1ui8);
	}
	SetBarrier(vkImgLayout, pVkCmd, vkDstStages, m_VkImageInfo.mipLevels - 1u);
}

void ImageBuffer::Reset()
{
	if (!m_VkMem) m_VkImage = VK_NULL_HANDLE;

	if (m_VkImage) vkDestroyImage(m_pVkDevice, m_VkImage, nullptr);
	if (m_VkDesc.imageView)
		vkDestroyImageView(m_pVkDevice, m_VkDesc.imageView, nullptr);
	if (m_VkDesc.sampler)
		vkDestroySampler(m_pVkDevice, m_VkDesc.sampler, nullptr);
	if (m_pVkCmd && m_VkCmdPool)
		vkFreeCommandBuffers(m_pVkDevice, m_VkCmdPool, 1u, &m_pVkCmd);
}

const VkImage &ImageBuffer::GetImage() const
{
	return m_VkImage;
}

const VkDescriptorImageInfo &ImageBuffer::GetDesc() const
{
	return m_VkDesc;
}

const VkFormat &ImageBuffer::GetFormat() const
{
	return m_VkImageInfo.format;
}

const VkCommandBuffer &ImageBuffer::GetCmd() const
{
	return m_pVkCmd;
}

void ImageBuffer::createImage(const VkImage vkImage, const VkFlags uReqProps)
{
	VkResult U_ASSERT_ONLY err;

	if (vkImage)
		// Swapchain
		m_VkImage = vkImage;
	else
	{
		// create image
		err = vkCreateImage(m_pVkDevice, &m_VkImageInfo, nullptr, &m_VkImage);
		assert(!err);

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(m_pVkDevice, m_VkImage, &memReqs);
		assert(!err);

		m_VkMemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		m_VkMemAlloc.pNext = nullptr;
		m_VkMemAlloc.allocationSize = memReqs.size;
		m_VkMemAlloc.memoryTypeIndex = 0u;

		bool U_ASSERT_ONLY pass = memTypeFromProps(m_VkMemProps,
			memReqs.memoryTypeBits, uReqProps, /* No requirements */
			m_VkMemAlloc.memoryTypeIndex);
		assert(pass);

		// allocate memory
		err = vkAllocateMemory(m_pVkDevice, &m_VkMemAlloc, nullptr,
			&m_VkMem);
		assert(!err);

		// bind memory
		err = vkBindImageMemory(m_pVkDevice, m_VkImage, m_VkMem, 0);
		assert(!err);
	}
}

void ImageBuffer::createView()
{
	auto view = VkImageViewCreateInfo
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,		//.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,										//.pNext = NULL,
		0u,												//.flags = 0,
		m_VkImage,										//.image = VK_NULL_HANDLE,
		VK_IMAGE_VIEW_TYPE_2D,							//.viewType = VK_IMAGE_VIEW_TYPE_2D,
		m_VkImageInfo.format,							//.format = depth_format,
														//.components = { 0 }
														//.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
														//.baseMipLevel = 0,
														//.levelCount = 1,
														//.baseArrayLayer = 0,
														//.layerCount = 1},
	};

	if (m_VkImageInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		view.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		view.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 1u, 0u, 1u };
	}
	else
	{
		view.components = {
			VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
		};
		view.subresourceRange = {
			VK_IMAGE_ASPECT_COLOR_BIT, 0u,
			VK_REMAINING_MIP_LEVELS, 0u, 1u
		};
	}

	/* create image view */
	VkResult U_ASSERT_ONLY err =
		vkCreateImageView(m_pVkDevice, &view, nullptr, &m_VkDesc.imageView);
	assert(!err);
}
