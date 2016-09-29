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
	m_vvVkBindings(uNumSet)
{
	m_vvVkBindings.shrink_to_fit();
	if (uNumSet > 0ui8)
		assert(m_vvVkBindings.data());
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
		{
			if (vVkDescPoolSizes.size() > 0 &&
				vVkDescPoolSizes.back().type == vkBinding.descriptorType)
				vVkDescPoolSizes.back().descriptorCount += vkBinding.descriptorCount;
			else
				vVkDescPoolSizes.push_back({ vkBinding.descriptorType, vkBinding.descriptorCount });
		}

	const VkDescriptorPoolCreateInfo descPool =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,										//.pNext = NULL,
		0u,												//.flags
		uint32_t(m_vvVkBindings.size()),				//.maxSets = 1,
		uint32_t(vVkDescPoolSizes.size()),				//.poolSizeCount = 2,
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
			uint32_t(m_vvVkBindings[i].size()),						//.bindingCount = 2,
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
		uint32_t(m_vVkDescSets.size()),					//.descriptorSetCount = 1,
		m_vVkDescLayouts.data()							//.pSetLayouts = &demo->desc_layout
	};
	err = vkAllocateDescriptorSets(m_pVkDevice, &allocInfo, m_vVkDescSets.data());
	assert(!err);
}

void DescSet::Reset()
{
	if (m_vVkDescSets.size() > 0 && m_VkDescPool)
		vkFreeDescriptorSets(m_pVkDevice, m_VkDescPool, uint32_t(m_vVkDescSets.size()), m_vVkDescSets.data());
	if (m_VkDescPool)
		vkDestroyDescriptorPool(m_pVkDevice, m_VkDescPool, nullptr);
	for (auto &vkDescLayout : m_vVkDescLayouts)
		if (vkDescLayout) vkDestroyDescriptorSetLayout(m_pVkDevice, vkDescLayout, nullptr);

	m_vvVkBindings.clear();
}

void DescSet::SetNumBindings(const uint8_t uSet, const uint8_t uNum)
{
	if (uint8_t(m_vvVkBindings.size()) < uSet + 1ui8)
		m_vvVkBindings.resize(uSet + 1);

	m_vvVkBindings[uSet].resize(uNum);
	m_vvVkBindings[uSet].shrink_to_fit();
	assert(m_vvVkBindings[uSet].data());
}

void DescSet::AttachBindings(const uint8_t uSet, const uint8_t uNum,
	const VkDescriptorType vkType, const VkShaderStageFlags vkStage,
	const bool bSeparate)
{
	if (uint8_t(m_vvVkBindings.size()) < uSet + 1ui8)
		m_vvVkBindings.resize(uSet + 1);

	const auto uNumBinding = bSeparate ? uNum : 1u;
	const auto uNumDesc = bSeparate ? 1u : uNum;

	for (auto i = 0ui8; i < uNumBinding; ++i)
	{
		const auto &vkLastBinding = m_vvVkBindings[uSet].back();
		const auto uBinding = m_vvVkBindings[uSet].size() > 0 ?
			vkLastBinding.binding + vkLastBinding.descriptorCount : 0u;

		const auto vkBinding = VkDescriptorSetLayoutBinding
		{
			uBinding,	//.binding = 0,
			vkType,		//.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			uNumDesc,	//.descriptorCount = 1,
			vkStage,	//.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			nullptr		//.pImmutableSamplers = NULL,
		};
		m_vvVkBindings[uSet].push_back(vkBinding);
		assert(m_vvVkBindings[uSet].data());
	}
}

void XSVK::DescSet::SetBindings(const uint8_t uSet, const uint8_t uStart,
	const uint8_t uNum, const VkDescriptorType vkType,
	const VkShaderStageFlags vkStage, const bool bSeparate)
{
	const auto uNumBinding = bSeparate ? uNum : 1u;
	const auto uNumDesc = bSeparate ? 1u : uNum;

	for (auto i = 0ui8; i < uNumBinding; ++i)
	{
		const auto uBinding = uStart + i;
		m_vvVkBindings[uSet][uBinding].binding = uBinding;
		m_vvVkBindings[uSet][uBinding].descriptorType = vkType;
		m_vvVkBindings[uSet][uBinding].descriptorCount = uNumDesc;
		m_vvVkBindings[uSet][uBinding].stageFlags = vkStage;
		m_vvVkBindings[uSet][uBinding].pImmutableSamplers = nullptr;
	}
}

void XSVK::DescSet::SetBuffers(const uint8_t uSet, const uint8_t uBinding,
	const ppBuffer ppBuffers)
{
	const auto &uNum = m_vvVkBindings[uSet][uBinding].descriptorCount;

	vector<VkDescriptorBufferInfo> vBufferDescs(uNum);
	assert(vBufferDescs.data());
	for (auto i = 0u; i < uNum; ++i)
		vBufferDescs[i] = ppBuffers[i]->GetDesc();

	VkWriteDescriptorSet write;
	memset(&write, 0, sizeof(write));
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_vVkDescSets[uSet];
	write.dstBinding = uBinding;
	write.descriptorCount = uNum;
	write.descriptorType = m_vvVkBindings[uSet][uBinding].descriptorType;
	write.pBufferInfo = vBufferDescs.data();
	
	vkUpdateDescriptorSets(m_pVkDevice, 1u, &write, 0u, nullptr);
}

void XSVK::DescSet::SetBuffers(const uint8_t uSet, const uint8_t uStart,
	const uint8_t uCount, const ppBuffer ppBuffers)
{
	vector<vector<VkDescriptorBufferInfo>> vvBufferDescs(0);
	vector<VkWriteDescriptorSet> vWrites(0);
	auto uBase = 0ui8;

	for (auto i = uStart; uBase < uCount; ++i)
	{
		const auto &uNum = m_vvVkBindings[uSet][i].descriptorCount;
		assert(uBase + uNum <= uCount);
		vvBufferDescs.push_back(vector<VkDescriptorBufferInfo>(uNum));
		assert(vvBufferDescs.data());
		assert(vvBufferDescs.back().data());
		auto &vBufferDescs = vvBufferDescs.back();
		for (auto j = 0u; j < uNum; ++j)
			vBufferDescs[j] = ppBuffers[uBase + j]->GetDesc();

		vWrites.push_back(VkWriteDescriptorSet());
		auto& write = vWrites.back();
		memset(&write, 0, sizeof(write));
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_vVkDescSets[uSet];
		write.dstBinding = i;
		write.descriptorCount = uNum;
		write.descriptorType = m_vvVkBindings[uSet][i].descriptorType;
		write.pBufferInfo = vBufferDescs.data();

		uBase += uNum;
	}

	vkUpdateDescriptorSets(m_pVkDevice, uint32_t(vWrites.size()),
		vWrites.data(), 0u, nullptr);
}

void DescSet::SetTextures(const uint8_t uSet, const uint8_t uBinding,
	const ppTexture ppTextures)
{
	const auto &uNum = m_vvVkBindings[uSet][uBinding].descriptorCount;

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	assert(vTexDescs.data());
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();

	VkWriteDescriptorSet write;
	memset(&write, 0, sizeof(write));
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_vVkDescSets[uSet];
	write.dstBinding = uBinding;
	write.descriptorCount = uNum;
	write.descriptorType = m_vvVkBindings[uSet][uBinding].descriptorType;
	write.pImageInfo = vTexDescs.data();
	
	vkUpdateDescriptorSets(m_pVkDevice, 1u, &write, 0u, nullptr);
}

void DescSet::SetTextures(const uint8_t uSet, const uint8_t uBinding,
	const ppImage ppTextures)
{
	const auto &uNum = m_vvVkBindings[uSet][uBinding].descriptorCount;

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	assert(vTexDescs.data());
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();

	VkWriteDescriptorSet write;
	memset(&write, 0, sizeof(write));
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_vVkDescSets[uSet];
	write.dstBinding = uBinding;
	write.descriptorCount = uNum;
	write.descriptorType = m_vvVkBindings[uSet][uBinding].descriptorType;
	write.pImageInfo = vTexDescs.data();
	
	vkUpdateDescriptorSets(m_pVkDevice, 1u, &write, 0u, nullptr);
}

void DescSet::SetTextures(const uint8_t uSet, const uint8_t uStart,
	const uint8_t uCount, const ppTexture ppTextures)
{
	vector<vector<VkDescriptorImageInfo>> vvTexDescs(0);
	vector<VkWriteDescriptorSet> vWrites(0);
	auto uBase = 0ui8;

	for (auto i = uStart; uBase < uCount; ++i)
	{
		const auto &uNum = m_vvVkBindings[uSet][i].descriptorCount;
		assert(uBase + uNum <= uCount);
		vvTexDescs.push_back(vector<VkDescriptorImageInfo>(uNum));
		assert(vvTexDescs.data());
		assert(vvTexDescs.back().data());
		auto &vTexDescs = vvTexDescs.back();
		for (auto j = 0u; j < uNum; ++j)
			vTexDescs[j] = ppTextures[uBase + j]->GetDesc();

		vWrites.push_back(VkWriteDescriptorSet());
		auto& write = vWrites.back();
		memset(&write, 0, sizeof(write));
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_vVkDescSets[uSet];
		write.dstBinding = i;
		write.descriptorCount = uNum;
		write.descriptorType = m_vvVkBindings[uSet][i].descriptorType;
		write.pImageInfo = vTexDescs.data();

		uBase += uNum;
	}

	vkUpdateDescriptorSets(m_pVkDevice, uint32_t(vWrites.size()),
		vWrites.data(), 0u, nullptr);
}

void DescSet::SetTextures(const uint8_t uSet, const uint8_t uStart,
	const uint8_t uCount, const ppImage ppTextures)
{
	vector<vector<VkDescriptorImageInfo>> vvTexDescs(0);
	vector<VkWriteDescriptorSet> vWrites(0);
	auto uBase = 0ui8;

	for (auto i = uStart; uBase < uCount; ++i)
	{
		const auto &uNum = m_vvVkBindings[uSet][i].descriptorCount;
		assert(uBase + uNum <= uCount);
		vvTexDescs.push_back(vector<VkDescriptorImageInfo>(uNum));
		assert(vvTexDescs.data());
		assert(vvTexDescs.back().data());
		auto &vTexDescs = vvTexDescs.back();
		for (auto j = 0u; j < uNum; ++j)
			vTexDescs[j] = ppTextures[uBase + j]->GetDesc();

		vWrites.push_back(VkWriteDescriptorSet());
		auto& write = vWrites.back();
		memset(&write, 0, sizeof(write));
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_vVkDescSets[uSet];
		write.dstBinding = i;
		write.descriptorCount = uNum;
		write.descriptorType = m_vvVkBindings[uSet][i].descriptorType;
		write.pImageInfo = vTexDescs.data();

		uBase += uNum;
	}

	vkUpdateDescriptorSets(m_pVkDevice, uint32_t(vWrites.size()),
		vWrites.data(), 0u, nullptr);
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
