//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <type_traits>
#include "XSVKBuffer.h"

namespace XSVK
{
	class StorageBuffer :
		public Buffer
	{
	public:
		using pbyte = std::add_pointer_t<uint8_t>;

		StorageBuffer(const VkDevice pVkDevice,
			const VkPhysicalDeviceMemoryProperties &vkMemProps);
		virtual ~StorageBuffer();

		void Create(const size_t uSize, const pbyte pInitData = nullptr);
		void Reset();

		const VkDescriptorBufferInfo &GetDesc() const;
	protected:
		VkDescriptorBufferInfo	m_VkDesc;
	};
}
