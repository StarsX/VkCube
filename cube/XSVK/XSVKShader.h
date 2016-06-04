//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include <cassert>
#include <memory>
#include <vulkan/vulkan.h>

#define	MAX_SHADER_NUM			64

namespace XSVK
{
	using upuints = std::unique_ptr<uint32_t[]>;

	class Shader
	{
	public:
		using pchar = std::add_pointer_t<const char>;

		Shader(const VkDevice pVkDevice);
		virtual ~Shader();

		void CreateVertexShader(const uint8_t i,
			const pchar szFilename,
			const pchar szMain = "main");
		void CreateFragmentShader(const uint8_t i,
			const pchar szFilename,
			const pchar szMain = "main");
		void Reset();

		const VkPipelineShaderStageCreateInfo &GetVertexShader(const uint8_t i) const;
		const VkPipelineShaderStageCreateInfo &GetFragmentShader(const uint8_t i) const;

	protected:
		using puint = std::add_pointer_t<uint32_t>;

		upuints readSPV(const pchar szFilename, size_t &uSize);
		VkShaderModule createShader(const pchar szFilename);
		VkShaderModule createShader(const puint uCode, const size_t uSize);

		VkPipelineShaderStageCreateInfo m_pVertexShaders[MAX_SHADER_NUM];
		VkPipelineShaderStageCreateInfo m_pFragmentShaders[MAX_SHADER_NUM];

		const VkDevice					m_pVkDevice;
	};
}
