//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKIABuffer.h"

using namespace XSVK;

IABuffer::IABuffer(const VkDevice pVkDevice,
	const VkPhysicalDeviceMemoryProperties &vkMemProps) :
	Buffer(pVkDevice, vkMemProps),
	m_VkBuffer(VK_NULL_HANDLE)
{
}

IABuffer::~IABuffer()
{
	Reset();
}

void IABuffer::Create(const size_t uSize, const lpvoid pInitData,
	const VkBufferUsageFlags vkUsage, const VkFlags uReqProps)
{
	VkBufferCreateInfo bufInfo;
	memset(&bufInfo, 0, sizeof(bufInfo));
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.usage = vkUsage;
	bufInfo.size = uSize;
	VkResult U_ASSERT_ONLY err = vkCreateBuffer(
		m_pVkDevice, &bufInfo, nullptr, &m_VkBuffer);
	assert(!err);

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_pVkDevice, m_VkBuffer, &memReqs);

	m_VkMemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	m_VkMemAlloc.pNext = nullptr;
	m_VkMemAlloc.allocationSize = memReqs.size;
	m_VkMemAlloc.memoryTypeIndex = 0;

	bool U_ASSERT_ONLY bPass = memTypeFromProps(
		m_VkMemProps, memReqs.memoryTypeBits,
		uReqProps, m_VkMemAlloc.memoryTypeIndex);
	assert(bPass);
	
	err = vkAllocateMemory(m_pVkDevice, &m_VkMemAlloc, nullptr, &m_VkMem);
	assert(!err);

	if (pInitData) {
		lpvoid pData;
		Map(&pData);
		memcpy(pData, pInitData, uSize);
		Unmap();
	}

	err = vkBindBufferMemory(m_pVkDevice, m_VkBuffer, m_VkMem, 0);
	assert(!err);
}

void IABuffer::Reset()
{
	if (m_VkBuffer)
		vkDestroyBuffer(m_pVkDevice, m_VkBuffer, nullptr);
}

const VkBuffer &IABuffer::Get() const
{
	return m_VkBuffer;
}
