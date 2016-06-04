//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKBuffer.h"

using namespace XSVK;

Buffer::Buffer(const VkDevice pVkDevice,
	const VkPhysicalDeviceMemoryProperties &vkMemProps) :
	m_pVkDevice(pVkDevice),
	m_VkMemProps(vkMemProps),
	m_VkMem(VK_NULL_HANDLE)
{
	m_VkDesc.buffer = VK_NULL_HANDLE;
}

Buffer::~Buffer()
{
	Reset();
}

void Buffer::Create(const size_t uSize, const lpvoid pInitData,
	const VkBufferUsageFlags usage, const VkFlags uReqProps)
{
	VkBufferCreateInfo bufInfo;
	memset(&bufInfo, 0, sizeof(bufInfo));
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.usage = usage;
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

	err = vkBindBufferMemory(m_pVkDevice, m_VkDesc.buffer, m_VkMem, 0);
	assert(!err);

	m_VkDesc.offset = 0;
	m_VkDesc.range = uSize;
}

void Buffer::Map(ppvoid ppData)
{
	VkResult U_ASSERT_ONLY err =
		vkMapMemory(m_pVkDevice, m_VkMem, 0, m_VkMemAlloc.allocationSize, 0, ppData);
	assert(!err);
}

void Buffer::Unmap()
{
	vkUnmapMemory(m_pVkDevice, m_VkMem);
}

void Buffer::Reset()
{
	if (m_VkDesc.buffer) vkDestroyBuffer(m_pVkDevice, m_VkDesc.buffer, nullptr);
	if (m_VkMem) vkFreeMemory(m_pVkDevice, m_VkMem, nullptr);
}

const VkDescriptorBufferInfo &Buffer::GetDesc() const
{
	return m_VkDesc;
}

bool Buffer::memTypeFromProps(const VkPhysicalDeviceMemoryProperties &vkMemProps,
	uint32_t uTypeBits, const VkFlags reqMask, uint32_t &uTypeIndex)
{
	// Search memtypes to find first index with those properties
	for (auto i = 0u; i < 32u; ++i)
	{
		if ((uTypeBits & 1) == 1)
		{
			// Type is available, does it match user properties?
			if ((vkMemProps.memoryTypes[i].propertyFlags &
				reqMask) == reqMask)
			{
				uTypeIndex = i;
				return true;
			}
		}
		uTypeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}
