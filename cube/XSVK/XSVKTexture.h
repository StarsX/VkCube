//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <functional>
#include "XSVKImageBuffer.h"

namespace XSVK
{
	class Texture :
		public ImageBuffer
	{
	public:
		using lpvoid = std::add_pointer_t<void>;
		using FxLoadTexture = std::function<void(lpvoid, VkSubresourceLayout&)>;

		Texture(const VkDevice pVkDevice,
			const VkPhysicalDeviceMemoryProperties &vkMemProps);

		void Create(const VkCommandPool &vkCmdPool,
			const bool bStaging = true, const bool bSampler = true);
		void CreateImage(const VkFlags uReqProps);
		void SetGPUQueue(const VkPhysicalDevice vkGPU, const VkQueue pVkQueue);
		void SetLoadFunc(const FxLoadTexture &fxLoadData);

	protected:
		void initCmd(const VkCommandPool &vkCmdPool);
		void flushInitCmd();

		VkPhysicalDevice	m_pVkGPU;
		VkQueue				m_pVkQueue;
		FxLoadTexture		m_FxLoadData;
	};
}
