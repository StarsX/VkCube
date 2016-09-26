//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKDescSet.h"

using namespace std;
using namespace XSVK;

DescSet::DescSet(const VkDevice pVkDevice, const uint8_t uNumSet) :
	m_pVkDevice(pVkDevice),
	m_VkDescPool(VK_NULL_HANDLE),
	m_vVkDescLayouts(0),
	m_vVkDescSets(0),
	m_vvVkBindings(uNumSet),
	m_vVkWrites(0),
	m_vvBufferDesc(0),
	m_vvImageDesc(0)
{
	m_vvVkBindings.shrink_to_fit();
}

DescSet::~DescSet()
{
	Reset();
}

void DescSet::Create()
{
	CreateDescPool();
	CreateDescLayouts();
	CreateDescSets();
}

void DescSet::CreateDescPool()
{
	vector<VkDescriptorPoolSize> vVkDescPoolSizes(0);
	for (const auto &vVkBindings : m_vvVkBindings)
		for (const auto &vkBinding : vVkBindings)
			vVkDescPoolSizes.push_back({ vkBinding.descriptorType, vkBinding.descriptorCount });

	const VkDescriptorPoolCreateInfo descPool =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,										//.pNext = NULL,
		0u,												//.flags
		m_vvVkBindings.size(),							//.maxSets = 1,
		vVkDescPoolSizes.size(),						//.poolSizeCount = 2,
		vVkDescPoolSizes.data(),						//.pPoolSizes = type_counts,
	};
	VkResult U_ASSERT_ONLY err;

	err = vkCreateDescriptorPool(m_pVkDevice, &descPool, nullptr, &m_VkDescPool);
	assert(!err);
}

void DescSet::CreateDescLayouts()
{
	m_vVkDescLayouts.resize(m_vvVkBindings.size());
	m_vVkDescLayouts.shrink_to_fit();

	const auto uNum = uint8_t(m_vVkDescLayouts.size());

	for (auto i = 0ui8; i < uNum; ++i)
	{
		const auto descLayout = VkDescriptorSetLayoutCreateInfo
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			nullptr,												//.pNext = NULL,
			0u,														//.flags = 0,
			m_vvVkBindings[i].size(),								//.bindingCount = 2,
			m_vvVkBindings[i].data(),								//.pBindings = layout_bindings,
		};
		VkResult U_ASSERT_ONLY err = vkCreateDescriptorSetLayout(
			m_pVkDevice, &descLayout, nullptr, &m_vVkDescLayouts[i]);
		assert(!err);
	}
}

void DescSet::CreateDescSets()
{
	VkResult U_ASSERT_ONLY err;

	m_vVkDescSets.resize(m_vvVkBindings.size());
	m_vVkDescSets.shrink_to_fit();

	VkDescriptorSetAllocateInfo allocInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,										//.pNext = NULL,
		m_VkDescPool,									//.descriptorPool = demo->desc_pool,
		m_vVkDescSets.size(),							//.descriptorSetCount = 1,
		m_vVkDescLayouts.data()							//.pSetLayouts = &demo->desc_layout
	};
	err = vkAllocateDescriptorSets(m_pVkDevice, &allocInfo, m_vVkDescSets.data());
	assert(!err);

	for (auto &write : m_vVkWrites) write.dstSet = m_vVkDescSets[uint8_t(write.dstSet)];
	vkUpdateDescriptorSets(m_pVkDevice, uint32_t(m_vVkWrites.size()),
		m_vVkWrites.data(), 0u, nullptr);
}

void XSVK::DescSet::Update()
{
	for (auto &write : m_vVkWrites) write.dstSet = m_vVkDescSets[uint8_t(write.dstSet)];
	vkUpdateDescriptorSets(m_pVkDevice, uint32_t(m_vVkWrites.size()),
		m_vVkWrites.data(), 0u, nullptr);
}

void DescSet::Reset()
{
	if (m_vVkDescSets.size() > 0 && m_VkDescPool)
		vkFreeDescriptorSets(m_pVkDevice, m_VkDescPool, m_vVkDescSets.size(), m_vVkDescSets.data());
	if (m_VkDescPool)
		vkDestroyDescriptorPool(m_pVkDevice, m_VkDescPool, nullptr);
	for (auto &vkDescLayout : m_vVkDescLayouts)
		if (vkDescLayout) vkDestroyDescriptorSetLayout(m_pVkDevice, vkDescLayout, nullptr);

	m_vvVkBindings.clear();
	m_vVkWrites.clear();
	m_vvBufferDesc.clear();
	m_vvImageDesc.clear();
}

void DescSet::SetNumBindings(const uint8_t s, const uint8_t uNum)
{
	if (uint8_t(m_vvVkBindings.size()) < s + 1ui8)
		m_vvVkBindings.resize(s + 1);

	m_vvVkBindings[s].resize(uNum);
	m_vvVkBindings[s].shrink_to_fit();
	assert(m_vvVkBindings[s].data());
	m_vVkWrites.resize(uNum);
	m_vVkWrites.shrink_to_fit();
	assert(m_vVkWrites.data());
}

void DescSet::SetBuffers(const uint8_t s, const uint8_t i, const uint8_t uNum,
	const ppBuffer pBuffers, const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	setBindings(s, i, uNum, vkStage, vkType);

	vector<VkDescriptorBufferInfo> vBufferDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vBufferDescs[i] = pBuffers[i]->GetDesc();
	m_vvBufferDesc.push_back(vBufferDescs);
	assert(m_vvBufferDesc.data());
	
	memset(&m_vVkWrites[i], 0, sizeof(m_vVkWrites[i]));
	m_vVkWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	m_vVkWrites[i].dstBinding = m_vvVkBindings[s][i].binding;
	m_vVkWrites[i].descriptorCount = uNum;
	m_vVkWrites[i].descriptorType = vkType;
	m_vVkWrites[i].pBufferInfo = m_vvBufferDesc[m_vvBufferDesc.size() - 1].data();
	m_vVkWrites[i].dstSet = s;
}

void DescSet::AttachBuffers(const uint8_t s, const uint8_t uNum,
	const ppBuffer pBuffers, const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	if (uint8_t(m_vvVkBindings.size()) < s + 1ui8)
		m_vvVkBindings.resize(s + 1);

	const auto uBinding = uint32_t(m_vvVkBindings[s].size());
	const auto vkBinding = VkDescriptorSetLayoutBinding
	{
		uBinding,	//.binding = 0,
		vkType,		//.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		uNum,		//.descriptorCount = 1,
		vkStage,	//.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		nullptr		//.pImmutableSamplers = NULL,
	};
	m_vvVkBindings[s].push_back(vkBinding);
	assert(m_vvVkBindings[s].data());

	vector<VkDescriptorBufferInfo> vBufferDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vBufferDescs[i] = pBuffers[i]->GetDesc();
	m_vvBufferDesc.push_back(vBufferDescs);
	assert(m_vvBufferDesc.data());

	VkWriteDescriptorSet write;
	memset(&write, 0, sizeof(write));
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = uBinding;
	write.descriptorCount = uNum;
	write.descriptorType = vkType;
	write.pBufferInfo = m_vvBufferDesc[m_vvBufferDesc.size() - 1].data();
	write.dstSet = s;
	m_vVkWrites.push_back(write);
	assert(m_vVkWrites.data());
}

void DescSet::SetTextures(const uint8_t s, const uint8_t i, const uint8_t uNum,
	const ppTexture ppTextures, const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	setBindings(s, i, uNum, vkStage, vkType);

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();
	m_vvImageDesc.push_back(vTexDescs);
	assert(m_vvImageDesc.data());

	memset(&m_vVkWrites[i], 0, sizeof(m_vVkWrites[i]));
	m_vVkWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	m_vVkWrites[i].dstBinding = m_vvVkBindings[s][i].binding;
	m_vVkWrites[i].descriptorCount = uNum;
	m_vVkWrites[i].descriptorType = vkType;
	m_vVkWrites[i].pImageInfo = m_vvImageDesc[m_vvImageDesc.size() - 1].data();
	m_vVkWrites[i].dstSet = s;
}

void DescSet::SetTextures(const uint8_t s, const uint8_t i, const uint8_t uNum,
	const ppImage ppTextures, const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	setBindings(s, i, uNum, vkStage, vkType);

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();
	m_vvImageDesc.push_back(vTexDescs);
	assert(m_vvImageDesc.data());

	memset(&m_vVkWrites[i], 0, sizeof(m_vVkWrites[i]));
	m_vVkWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	m_vVkWrites[i].dstBinding = m_vvVkBindings[s][i].binding;
	m_vVkWrites[i].descriptorCount = uNum;
	m_vVkWrites[i].descriptorType = vkType;
	m_vVkWrites[i].pImageInfo = m_vvImageDesc[m_vvImageDesc.size() - 1].data();
	m_vVkWrites[i].dstSet = s;
}

void DescSet::AttachTextures(const uint8_t s, const uint8_t uNum,
	const ppTexture ppTextures, const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	if (uint8_t(m_vvVkBindings.size()) < s + 1ui8)
		m_vvVkBindings.resize(s + 1);

	const auto uBinding = uint32_t(m_vvVkBindings[s].size());
	const auto vkBinding = VkDescriptorSetLayoutBinding
	{
		uBinding,	//.binding = 1,
		vkType,		//.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		uNum,		//.descriptorCount = DEMO_TEXTURE_COUNT,
		vkStage,	//.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		nullptr		//.pImmutableSamplers = NULL,
	};
	m_vvVkBindings[s].push_back(vkBinding);
	assert(m_vvVkBindings[s].data());

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();
	m_vvImageDesc.push_back(vTexDescs);
	assert(m_vvImageDesc.data());

	VkWriteDescriptorSet write;
	memset(&write, 0, sizeof(write));
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = uBinding;
	write.descriptorCount = uNum;
	write.descriptorType = vkType;
	write.pImageInfo = m_vvImageDesc[m_vvImageDesc.size() - 1].data();
	write.dstSet = s;
	m_vVkWrites.push_back(write);
	assert(m_vVkWrites.data());
}

void DescSet::AttachTextures(const uint8_t s, const uint8_t uNum,
	const ppImage ppTextures, const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	if (uint8_t(m_vvVkBindings.size()) < s + 1ui8)
		m_vvVkBindings.resize(s + 1);

	const auto uBinding = uint32_t(m_vvVkBindings[s].size());
	const auto vkBinding = VkDescriptorSetLayoutBinding
	{
		uBinding,	//.binding = 1,
		vkType,		//.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		uNum,		//.descriptorCount = DEMO_TEXTURE_COUNT,
		vkStage,	//.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		nullptr		//.pImmutableSamplers = NULL,
	};
	m_vvVkBindings[s].push_back(vkBinding);
	assert(m_vvVkBindings[s].data());

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();
	m_vvImageDesc.push_back(vTexDescs);
	assert(m_vvImageDesc.data());

	VkWriteDescriptorSet write;
	memset(&write, 0, sizeof(write));
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = uBinding;
	write.descriptorCount = uNum;
	write.descriptorType = vkType;
	write.pImageInfo = m_vvImageDesc[m_vvImageDesc.size() - 1].data();
	write.dstSet = s;
	m_vVkWrites.push_back(write);
	assert(m_vVkWrites.data());
}

const pVkDescriptorSet DescSet::Get() const
{
	return m_vVkDescSets.data();
}

const vVkDescriptorSetLayout &DescSet::GetLayouts() const
{
	return m_vVkDescLayouts;
}

const VkDescriptorSet &DescSet::Get(const uint8_t i) const
{
	return m_vVkDescSets[i];
}

const VkDescriptorSetLayout &DescSet::GetLayout(const uint8_t i) const
{
	return m_vVkDescLayouts[i];
}

void XSVK::DescSet::setBindings(const uint8_t s, const uint8_t i, const uint8_t uNum,
	const VkShaderStageFlags vkStage, const VkDescriptorType vkType)
{
	if (vkStage)
	{
		m_vvVkBindings[s][i].descriptorType = vkType;
		m_vvVkBindings[s][i].binding = i;
		m_vvVkBindings[s][i].descriptorCount = uNum;
		m_vvVkBindings[s][i].stageFlags = vkStage;
		m_vvVkBindings[s][i].pImmutableSamplers = nullptr;
	}
}
