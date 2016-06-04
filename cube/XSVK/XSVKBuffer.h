//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <cassert>
#include <type_traits>
#include <vulkan/vulkan.h>

namespace XSVK
{
	class Buffer
	{
	public:
		using lpvoid = std::add_pointer_t<void>;
		using ppvoid = std::add_pointer_t<lpvoid>;

		Buffer(const VkDevice pVkDevice,
			const VkPhysicalDeviceMemoryProperties &vkMemProps);
		virtual ~Buffer();

		void Create(const size_t uSize, const lpvoid pInitData = nullptr,
			const VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			const VkFlags uReqProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		void Map(ppvoid ppData);
		void Unmap();
		void Reset();

		const VkDescriptorBufferInfo &GetDesc() const;

	protected:
		static bool memTypeFromProps(const VkPhysicalDeviceMemoryProperties &vkMemProps,
			uint32_t uTypeBits, const VkFlags reqMask, uint32_t &uTypeIndex);

		VkMemoryAllocateInfo					m_VkMemAlloc;
		VkDeviceMemory							m_VkMem;

		const VkDevice							m_pVkDevice;
		const VkPhysicalDeviceMemoryProperties	&m_VkMemProps;
	private:
		VkDescriptorBufferInfo					m_VkDesc;
	};
}
