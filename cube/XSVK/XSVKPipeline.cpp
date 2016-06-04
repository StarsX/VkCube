//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKPipeline.h"

using namespace std;
using namespace XSVK;

Pipeline::Pipeline(const VkDevice pVkDevice) :
	m_pVkDevice(pVkDevice),
	m_VkPipelineLayout(VK_NULL_HANDLE),
	m_VkPipelineCache(VK_NULL_HANDLE),
	m_VkPipeline(VK_NULL_HANDLE),
	m_vVkShaders(0),
	m_vVkBlendAtts(0),
	m_vVkBindings(0),
	m_vVkAttrDescs(0)
{
	memset(m_VkDynamics, 0, sizeof m_VkDynamics);

	memset(&m_VkInputAssemply, 0, sizeof(m_VkInputAssemply));
	m_VkInputAssemply.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_VkInputAssemply.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	memset(&m_VkRasterization, 0, sizeof(m_VkRasterization));
	m_VkRasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_VkRasterization.polygonMode = VK_POLYGON_MODE_FILL;
	m_VkRasterization.cullMode = VK_CULL_MODE_BACK_BIT;
	m_VkRasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	m_VkRasterization.depthClampEnable = VK_FALSE;
	m_VkRasterization.rasterizerDiscardEnable = VK_FALSE;
	m_VkRasterization.depthBiasEnable = VK_FALSE;
	m_VkRasterization.lineWidth = 1.0f;

	m_vVkBlendAtts.resize(1);
	m_vVkBlendAtts.shrink_to_fit();
	assert(m_vVkBlendAtts.data());
	memset(m_vVkBlendAtts.data(), 0, sizeof(VkPipelineColorBlendAttachmentState));
	m_vVkBlendAtts[0].colorWriteMask = 0xf;

	memset(&m_VkViewport, 0, sizeof(m_VkViewport));
	m_VkViewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_VkViewport.viewportCount = 1u;
	m_VkViewport.scissorCount = 1u;

	memset(&m_VkDepthStencil, 0, sizeof(m_VkDepthStencil));
	m_VkDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_VkDepthStencil.depthTestEnable = VK_TRUE;
	m_VkDepthStencil.depthWriteEnable = VK_TRUE;
	m_VkDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	m_VkDepthStencil.depthBoundsTestEnable = VK_FALSE;
	m_VkDepthStencil.back.failOp = VK_STENCIL_OP_KEEP;
	m_VkDepthStencil.back.passOp = VK_STENCIL_OP_KEEP;
	m_VkDepthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
	m_VkDepthStencil.stencilTestEnable = VK_FALSE;
	m_VkDepthStencil.front = m_VkDepthStencil.back;

	memset(&m_VkMultisample, 0, sizeof(m_VkMultisample));
	m_VkMultisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_VkMultisample.pSampleMask = nullptr;
	m_VkMultisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

Pipeline::~Pipeline()
{
	Reset();
}

void Pipeline::Create(
	const VkDescriptorSetLayout &pVkDescLayout,
	const VkRenderPass &pVkRenderPass)
{
	CreateLayout(pVkDescLayout);
	Create(pVkRenderPass);
}

void Pipeline::Create(const VkRenderPass &pVkRenderPass)
{
	VkPipelineDynamicStateCreateInfo dynamicState;
	memset(&dynamicState, 0, sizeof dynamicState);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = m_VkDynamics;

	VkPipelineVertexInputStateCreateInfo vertexInput;
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.pNext = nullptr;
	vertexInput.vertexBindingDescriptionCount = uint32_t(m_vVkBindings.size());
	vertexInput.vertexAttributeDescriptionCount = uint32_t(m_vVkAttrDescs.size());
	vertexInput.pVertexBindingDescriptions = m_vVkBindings.data();
	vertexInput.pVertexAttributeDescriptions = m_vVkAttrDescs.data();

	VkGraphicsPipelineCreateInfo pipeline;
	memset(&pipeline, 0, sizeof(pipeline));
	pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline.layout = m_VkPipelineLayout;

	VkPipelineColorBlendStateCreateInfo	blend;
	memset(&blend, 0, sizeof(blend));
	blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend.attachmentCount = uint32_t(m_vVkBlendAtts.size());
	blend.pAttachments = m_vVkBlendAtts.data();

	m_VkDynamics[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	m_VkDynamics[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	VkPipelineCacheCreateInfo pipelineCache;
	memset(&pipelineCache, 0, sizeof(pipelineCache));
	pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	VkResult U_ASSERT_ONLY err = vkCreatePipelineCache(m_pVkDevice,
		&pipelineCache, nullptr, &m_VkPipelineCache);
	assert(!err);

	pipeline.stageCount = uint32_t(m_vVkShaders.size());
	pipeline.pStages = m_vVkShaders.data();
	pipeline.pVertexInputState = &vertexInput;
	pipeline.pInputAssemblyState = &m_VkInputAssemply;
	pipeline.pRasterizationState = &m_VkRasterization;
	pipeline.pColorBlendState = &blend;
	pipeline.pMultisampleState = &m_VkMultisample;
	pipeline.pViewportState = &m_VkViewport;
	pipeline.pDepthStencilState = &m_VkDepthStencil;
	pipeline.renderPass = pVkRenderPass;
	pipeline.pDynamicState = &dynamicState;

	err = vkCreateGraphicsPipelines(m_pVkDevice, m_VkPipelineCache, 1u,
		&pipeline, nullptr, &m_VkPipeline);
	assert(!err);
}

void Pipeline::CreateLayout(const VkDescriptorSetLayout &pVkDescLayout)
{
	VkResult U_ASSERT_ONLY err;

	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,			//.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,												//.pNext = NULL,
		0u,														//.flags = 0,
		1u,														//.setLayoutCount = 1,
		&pVkDescLayout											//.pSetLayouts = &demo->desc_layout,
	};

	err = vkCreatePipelineLayout(m_pVkDevice, &pipelineLayoutCreateInfo, nullptr,
		&m_VkPipelineLayout);
	assert(!err);
}

void Pipeline::AttachShader(const VkPipelineShaderStageCreateInfo &vkShader)
{
	m_vVkShaders.push_back(vkShader);
	assert(m_vVkShaders.data());
}

void Pipeline::IASetNumBindings(const uint8_t uNum)
{
	m_vVkBindings.resize(uNum);
	m_vVkBindings.shrink_to_fit();
	assert(m_vVkBindings.data());
}

void Pipeline::IASetBindings(const uint8_t i, const uint8_t uBinding,
	const uint8_t uStride)
{
	m_vVkBindings[i].binding = uBinding;
	m_vVkBindings[i].stride = uStride;
	m_vVkBindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void Pipeline::IAAttachBindings(const uint8_t uBinding, const uint8_t uStride)
{
	const auto binding = VkVertexInputBindingDescription
	{
		uBinding,
		uStride,
		VK_VERTEX_INPUT_RATE_VERTEX
	};
	m_vVkBindings.push_back(binding);
	assert(m_vVkBindings.data());
}

void Pipeline::IASetNumAttributes(const uint8_t uNum)
{
	m_vVkAttrDescs.resize(uNum);
	m_vVkAttrDescs.shrink_to_fit();
	assert(m_vVkAttrDescs.data());
}

void Pipeline::IASetAttribute(const uint8_t i, const uint8_t uBinding,
	const VkFormat vkFormat, const uint8_t uOffset)
{
	m_vVkAttrDescs[i].location = i;
	m_vVkAttrDescs[i].binding = uBinding;
	m_vVkAttrDescs[i].format = vkFormat;
	m_vVkAttrDescs[i].offset = uOffset;
}

void Pipeline::IAAttachAttribute(const uint8_t uBinding,
	const VkFormat vkFormat, const uint8_t uOffset)
{
	const auto attribute = VkVertexInputAttributeDescription
	{
		uint32_t(m_vVkAttrDescs.size()),
		uBinding,
		vkFormat,
		uOffset
	};
	m_vVkAttrDescs.push_back(attribute);
	assert(m_vVkAttrDescs.data());
}

void Pipeline::IASetPrimitiveTopology(const VkPrimitiveTopology vkPrimTopology)
{
	m_VkInputAssemply.topology = vkPrimTopology;
}

void Pipeline::RSSetPolygonMode(const VkPolygonMode vkPolygonMode)
{
	m_VkRasterization.polygonMode = vkPolygonMode;
}

void Pipeline::RSSetCullMode(const VkCullModeFlags vkCullMode,
	const VkFrontFace vkFrontFace)
{
	m_VkRasterization.cullMode = vkCullMode;
	m_VkRasterization.frontFace = vkFrontFace;
}

void Pipeline::RSSetDepthBias(const VkBool32 vkDepthBias,
	const float fDepthBias, const float fDepthBiasClamp,
	const float fDepthBiasSlope)
{
	m_VkRasterization.depthBiasEnable = vkDepthBias;
	m_VkRasterization.depthBiasConstantFactor = fDepthBias;
	m_VkRasterization.depthBiasClamp = fDepthBiasClamp;
	m_VkRasterization.depthBiasSlopeFactor = fDepthBiasSlope;
}

void Pipeline::RSSetLineWidth(const float fLineWidth)
{
	m_VkRasterization.lineWidth = fLineWidth;
}

void Pipeline::RSSetSampleCount(const VkSampleCountFlagBits vkSampleCount)
{
	m_VkMultisample.rasterizationSamples = vkSampleCount;
}

void Pipeline::OMSetNumBlendAtts(uint8_t uNum)
{
	m_vVkBlendAtts.resize(uNum);
	m_vVkBlendAtts.shrink_to_fit();
	assert(m_vVkBlendAtts.data());
	for (auto i = 0ui8; i < uNum; ++i)
	{
		memset(&m_vVkBlendAtts[i], 0, sizeof(VkPipelineColorBlendAttachmentState));
		m_vVkBlendAtts[i].colorWriteMask = 0xf;
	}
}

void Pipeline::OMSetBlendColor(const uint8_t i, const VkBlendFactor vkSrc,
	const VkBlendFactor vkDst, const VkBlendOp vkOp)
{
	m_vVkBlendAtts[i].blendEnable = VK_TRUE;
	m_vVkBlendAtts[i].srcColorBlendFactor = vkSrc;
	m_vVkBlendAtts[i].dstColorBlendFactor = vkDst;
	m_vVkBlendAtts[i].colorBlendOp = vkOp;
}

void Pipeline::OMSetBlendAlpha(const uint8_t i, const VkBlendFactor vkSrc,
	const VkBlendFactor vkDst, const VkBlendOp vkOp)
{
	m_vVkBlendAtts[i].blendEnable = VK_TRUE;
	m_vVkBlendAtts[i].srcAlphaBlendFactor = vkSrc;
	m_vVkBlendAtts[i].dstAlphaBlendFactor = vkDst;
	m_vVkBlendAtts[i].alphaBlendOp = vkOp;
}

void Pipeline::OMAttachBlend(const VkBool32 vkEnable,
	const VkBlendFactor vkColorSrc, const VkBlendFactor vkColorDst,
	const VkBlendFactor vkAlphaSrc, const VkBlendFactor vkAlphaDst,
	const VkBlendOp vkColorOp, const VkBlendOp vkAlphaOp)
{
	VkPipelineColorBlendAttachmentState blendAtt;
	blendAtt.blendEnable = vkEnable;
	blendAtt.srcColorBlendFactor = vkColorSrc;
	blendAtt.dstColorBlendFactor = vkColorDst;
	blendAtt.srcAlphaBlendFactor = vkAlphaSrc;
	blendAtt.dstAlphaBlendFactor = vkAlphaDst;
	blendAtt.colorBlendOp = vkColorOp;
	blendAtt.alphaBlendOp = vkAlphaOp;
	blendAtt.colorWriteMask = 0xf;
	m_vVkBlendAtts.push_back(blendAtt);
	assert(m_vVkBlendAtts.data());
}

void Pipeline::OMSetDisableBlend(const uint8_t i)
{
	m_vVkBlendAtts[i].blendEnable = VK_FALSE;
}

void Pipeline::DSSetDepth(const VkBool32 vkEnable,
	const VkBool32 vkWriteEnable, VkCompareOp vkOp)
{
	m_VkDepthStencil.depthTestEnable = vkEnable;
	m_VkDepthStencil.depthWriteEnable = vkWriteEnable;
	m_VkDepthStencil.depthCompareOp = vkOp;
}

void Pipeline::Reset()
{
	if (m_VkPipeline)
		vkDestroyPipeline(m_pVkDevice, m_VkPipeline, nullptr);
	if (m_VkPipelineCache)
		vkDestroyPipelineCache(m_pVkDevice, m_VkPipelineCache, nullptr);
	if (m_VkPipelineLayout)
		vkDestroyPipelineLayout(m_pVkDevice, m_VkPipelineLayout, nullptr);

	m_vVkShaders.clear();
	m_vVkBlendAtts.clear();
	m_vVkBindings.clear();
	m_vVkAttrDescs.clear();
}

const VkPipeline &Pipeline::Get() const
{
	return m_VkPipeline;
}

const VkPipelineLayout &Pipeline::GetLayout() const
{
	return m_VkPipelineLayout;
}
