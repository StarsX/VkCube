//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKStorageBuffer.h"

using namespace XSVK;

StorageBuffer::StorageBuffer(const VkDevice pVkDevice,
	const VkPhysicalDeviceMemoryProperties &vkMemProps) :
	Buffer(pVkDevice, vkMemProps)
{
	m_VkDesc.buffer = VK_NULL_HANDLE;
}

StorageBuffer::~StorageBuffer()
{
	Reset();
}

void StorageBuffer::Create(const size_t uSize, const pbyte pInitData)
{
	VkBufferCreateInfo bufInfo;
	memset(&bufInfo, 0, sizeof(bufInfo));
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufInfo.size = uSize;
	VkResult U_ASSERT_ONLY err = vkCreateBuffer(m_pVkDevice,
		&bufInfo, nullptr, &m_VkDesc.buffer);
	assert(!err);

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_pVkDevice, m_VkDesc.buffer, &memReqs);

	m_VkMemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	m_VkMemAlloc.pNext = nullptr;
	m_VkMemAlloc.allocationSize = memReqs.size;
	m_VkMemAlloc.memoryTypeIndex = 0;

	bool U_ASSERT_ONLY bPass = memTypeFromProps(
		m_VkMemProps, memReqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		m_VkMemAlloc.memoryTypeIndex);
	assert(bPass);

	err = vkAllocateMemory(m_pVkDevice, &m_VkMemAlloc, nullptr, &m_VkMem);
	assert(!err);

	if (pInitData) {
		uint8_t *pData;
		Map((void **)&pData);
		memcpy(pData, pInitData, uSize);
		Unmap();
	}

	err = vkBindBufferMemory(m_pVkDevice, m_VkDesc.buffer, m_VkMem, 0);
	assert(!err);

	m_VkDesc.offset = 0;
	m_VkDesc.range = uSize;
}

void StorageBuffer::Reset()
{
	if (m_VkDesc.buffer) vkDestroyBuffer(m_pVkDevice, m_VkDesc.buffer, nullptr);
}

const VkDescriptorBufferInfo &StorageBuffer::GetDesc() const
{
	return m_VkDesc;
}
