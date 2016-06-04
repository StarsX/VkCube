//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include "XSVKBuffer.h"

namespace XSVK
{
	class IABuffer :
		public Buffer
	{
	public:
		IABuffer(const VkDevice pVkDevice,
			const VkPhysicalDeviceMemoryProperties &vkMemProps);
		virtual ~IABuffer();

		void Create(const size_t uSize, const lpvoid pInitData,
			const VkBufferUsageFlags vkUsage,
			const VkFlags uReqProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		void Reset();

		const VkBuffer	&Get() const;
	protected:
		VkBuffer		m_VkBuffer;
	};
}
