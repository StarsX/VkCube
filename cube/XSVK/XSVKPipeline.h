//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSVKShader.h"
#include "XSVKDescSet.h"
#include "XSVKRenderPass.h"

namespace XSVK
{
	class Pipeline
	{
	public:
		Pipeline(const VkDevice pVkDevice);
		virtual ~Pipeline();

		void Create(const VkRenderPass &pVkRenderPass,
			const vVkDescriptorSetLayout &vVkDescLayouts);
		void Create(const VkRenderPass pVkRenderPass,
			const pVkDescriptorSetLayout pVkDescLayouts,
			const uint8_t uNum = 1ui8);
		void Create(const VkRenderPass &pVkRenderPass);
		void CreateLayout(const pVkDescriptorSetLayout pVkDescLayouts,
			const uint8_t uNum);

		void AttachShader(const VkPipelineShaderStageCreateInfo &vkShader);

		void IASetNumBindings(const uint8_t uNum);
		void IASetBindings(const uint8_t i,
			const uint8_t uBinding, const uint8_t uStride);
		void IAAttachBindings(const uint8_t uBinding, const uint8_t uStride);
		void IASetNumAttributes(const uint8_t uNum);
		void IASetAttribute(const uint8_t i, const uint8_t uBinding,
			const VkFormat vkFormat, const uint8_t uOffset);
		void IAAttachAttribute(const uint8_t uBinding,
			const VkFormat vkFormat, const uint8_t uOffset);
		void IASetPrimitiveTopology(const VkPrimitiveTopology vkPrimTopology);

		void RSSetPolygonMode(const VkPolygonMode vkPolygonMode);
		void RSSetCullMode(const VkCullModeFlags vkCullMode,
			const VkFrontFace vkFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE);
		void RSSetDepthBias(const VkBool32 vkDepthBias, const float fDepthBias,
			const float fDepthBiasClamp, const float fDepthBiasSlope);
		void RSSetLineWidth(const float fLineWidth);
		void RSSetSampleCount(const VkSampleCountFlagBits vkSampleCount);

		void OMSetNumBlendAtts(uint8_t uNum);
		void OMSetBlendColor(const uint8_t i, const VkBlendFactor vkSrc,
			const VkBlendFactor vkDst, const VkBlendOp vkOp = VK_BLEND_OP_ADD);
		void OMSetBlendAlpha(const uint8_t i, const VkBlendFactor vkSrc,
			const VkBlendFactor vkDst, const VkBlendOp vkOp = VK_BLEND_OP_ADD);
		void OMAttachBlend(const VkBool32 vkEnable = VK_FALSE,
			const VkBlendFactor vkColorSrc = VK_BLEND_FACTOR_ONE,
			const VkBlendFactor vkColorDst = VK_BLEND_FACTOR_ZERO,
			const VkBlendFactor vkAlphaSrc = VK_BLEND_FACTOR_ONE,
			const VkBlendFactor vkAlphaDst = VK_BLEND_FACTOR_ZERO,
			const VkBlendOp vkColorOp = VK_BLEND_OP_ADD,
			const VkBlendOp vkAlphaOp = VK_BLEND_OP_ADD);
		void OMSetDisableBlend(const uint8_t i);

		void DSSetDepth(const VkBool32 vkEnable, const VkBool32 vkWriteEnable,
			const VkCompareOp vkOp = VK_COMPARE_OP_LESS_OR_EQUAL);
		//void SetStencil(const VkBool32 vkEnable);

		void Reset();

		const VkPipeline						&Get() const;
		const VkPipelineLayout					&GetLayout() const;

	protected:
		using vVkPipelineShaderStageCreateInfo = std::vector<VkPipelineShaderStageCreateInfo>;
		using vVkPipelineColorBlendAttachmentState = std::vector<VkPipelineColorBlendAttachmentState>;
		using vVkVertexInputBindingDescription = std::vector<VkVertexInputBindingDescription>;
		using vVkVertexInputAttributeDescription = std::vector<VkVertexInputAttributeDescription>;

		VkPipelineLayout						m_VkPipelineLayout;
		VkPipelineCache							m_VkPipelineCache;
		VkPipeline								m_VkPipeline;

		VkDynamicState							m_VkDynamics[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineInputAssemblyStateCreateInfo	m_VkInputAssemply;
		VkPipelineRasterizationStateCreateInfo	m_VkRasterization;
		VkPipelineViewportStateCreateInfo		m_VkViewport;
		VkPipelineDepthStencilStateCreateInfo	m_VkDepthStencil;
		VkPipelineMultisampleStateCreateInfo	m_VkMultisample;
		vVkPipelineShaderStageCreateInfo		m_vVkShaders;
		vVkPipelineColorBlendAttachmentState	m_vVkBlendAtts;
		vVkVertexInputBindingDescription		m_vVkBindings;
		vVkVertexInputAttributeDescription		m_vVkAttrDescs;

		const VkDevice							m_pVkDevice;
	};
}
