//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKDescSet.h"

using namespace std;
using namespace XSVK;

DescSet::DescSet(const VkDevice pVkDevice) :
	m_pVkDevice(pVkDevice),
	m_VkDescLayout(VK_NULL_HANDLE),
	m_VkDescPool(VK_NULL_HANDLE),
	m_VkDescSet(VK_NULL_HANDLE),
	m_vVkBindings(0),
	m_vVkWrites(0),
	m_vvBufferDesc(0),
	m_vvImageDesc(0)
{
}

DescSet::~DescSet()
{
	Reset();
}

void DescSet::Create()
{
	CreateDescLayout();
	CreateDescPool();
	CreateDescSet();
}

void DescSet::CreateDescLayout()
{
	const auto descLayout = VkDescriptorSetLayoutCreateInfo
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,												//.pNext = NULL,
		0u,														//.flags = 0,
		uint32_t(m_vVkBindings.size()),							//.bindingCount = 2,
		m_vVkBindings.data(),									//.pBindings = layout_bindings,
	};
	VkResult U_ASSERT_ONLY err = vkCreateDescriptorSetLayout(
		m_pVkDevice, &descLayout, nullptr, &m_VkDescLayout);
	assert(!err);
}

void DescSet::CreateDescPool()
{
	vector<VkDescriptorPoolSize> vVkDescPoolSizes(m_vVkBindings.size());
	const auto uNumPoolSize = uint32_t(vVkDescPoolSizes.size());
	for (auto i = 0u; i < uNumPoolSize; ++i)
	{
		vVkDescPoolSizes[i].type = m_vVkBindings[i].descriptorType;
		vVkDescPoolSizes[i].descriptorCount = m_vVkBindings[i].descriptorCount;
	}
	const VkDescriptorPoolCreateInfo descPool =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,										//.pNext = NULL,
		0u,												//.flags
		1u,												//.maxSets = 1,
		uNumPoolSize,									//.poolSizeCount = 2,
		vVkDescPoolSizes.data(),						//.pPoolSizes = type_counts,
	};
	VkResult U_ASSERT_ONLY err;

	err = vkCreateDescriptorPool(m_pVkDevice, &descPool, nullptr, &m_VkDescPool);
	assert(!err);
}

void DescSet::CreateDescSet()
{
	VkResult U_ASSERT_ONLY err;

	VkDescriptorSetAllocateInfo allocInfo =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		nullptr,										//.pNext = NULL,
		m_VkDescPool,									//.descriptorPool = demo->desc_pool,
		1u,												//.descriptorSetCount = 1,
		&m_VkDescLayout									//.pSetLayouts = &demo->desc_layout
	};
	err = vkAllocateDescriptorSets(m_pVkDevice, &allocInfo, &m_VkDescSet);
	assert(!err);

	for (auto &write : m_vVkWrites) write.dstSet = m_VkDescSet;
	vkUpdateDescriptorSets(m_pVkDevice, uint32_t(m_vVkWrites.size()),
		m_vVkWrites.data(), 0u, nullptr);
}

void DescSet::Reset()
{
	if (m_VkDescSet && m_VkDescPool)
		vkFreeDescriptorSets(m_pVkDevice, m_VkDescPool, 1u, &m_VkDescSet);
	if (m_VkDescPool)
		vkDestroyDescriptorPool(m_pVkDevice, m_VkDescPool, nullptr);
	if (m_VkDescLayout)
		vkDestroyDescriptorSetLayout(m_pVkDevice, m_VkDescLayout, nullptr);

	m_vVkBindings.clear();
	m_vVkWrites.clear();
	m_vvBufferDesc.clear();
	m_vvImageDesc.clear();
}

void DescSet::SetNumBindings(const uint8_t uNum)
{
	m_vVkBindings.resize(uNum);
	m_vVkBindings.shrink_to_fit();
	assert(m_vVkBindings.data());
	m_vVkWrites.resize(uNum);
	m_vVkWrites.shrink_to_fit();
	assert(m_vVkWrites.data());
}

void DescSet::SetBuffers(
	const uint8_t i, const uint8_t uNum,
	const ppBuffer pBuffers,
	const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	m_vVkBindings[i].descriptorType = vkType;
	m_vVkBindings[i].binding = i;
	m_vVkBindings[i].descriptorCount = uNum;
	m_vVkBindings[i].stageFlags = vkStage;
	m_vVkBindings[i].pImmutableSamplers = nullptr;

	vector<VkDescriptorBufferInfo> vBufferDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vBufferDescs[i] = pBuffers[i]->GetDesc();
	m_vvBufferDesc.push_back(vBufferDescs);
	assert(m_vvBufferDesc.data());
	
	memset(&m_vVkWrites[i], 0, sizeof(m_vVkWrites[i]));
	m_vVkWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	m_vVkWrites[i].dstBinding = m_vVkBindings[i].binding;
	m_vVkWrites[i].descriptorCount = uNum;
	m_vVkWrites[i].descriptorType = vkType;
	m_vVkWrites[i].pBufferInfo = m_vvBufferDesc[m_vvBufferDesc.size() - 1].data();
}

void DescSet::AttachBuffers(const uint8_t uNum,
	const ppBuffer pBuffers,
	const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	const auto uBinding = uint32_t(m_vVkBindings.size());
	const auto vkBinding = VkDescriptorSetLayoutBinding
	{
		uBinding,	//.binding = 0,
		vkType,		//.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		uNum,		//.descriptorCount = 1,
		vkStage,	//.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		nullptr		//.pImmutableSamplers = NULL,
	};
	m_vVkBindings.push_back(vkBinding);
	assert(m_vVkBindings.data());

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
	m_vVkWrites.push_back(write);
	assert(m_vVkWrites.data());
}

void DescSet::SetTextures(
	const uint8_t i, const uint8_t uNum,
	const ppTexture ppTextures,
	const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	m_vVkBindings[i].descriptorType = vkType;
	m_vVkBindings[i].binding = i;
	m_vVkBindings[i].descriptorCount = uNum;
	m_vVkBindings[i].stageFlags = vkStage;
	m_vVkBindings[i].pImmutableSamplers = nullptr;

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();
	m_vvImageDesc.push_back(vTexDescs);
	assert(m_vvImageDesc.data());

	memset(&m_vVkWrites[i], 0, sizeof(m_vVkWrites[i]));
	m_vVkWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	m_vVkWrites[i].dstBinding = m_vVkBindings[i].binding;
	m_vVkWrites[i].descriptorCount = uNum;
	m_vVkWrites[i].descriptorType = vkType;
	m_vVkWrites[i].pImageInfo = m_vvImageDesc[m_vvImageDesc.size() - 1].data();
}

void DescSet::SetTextures(
	const uint8_t i, const uint8_t uNum,
	const ppImage ppTextures,
	const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	m_vVkBindings[i].descriptorType = vkType;
	m_vVkBindings[i].binding = i;
	m_vVkBindings[i].descriptorCount = uNum;
	m_vVkBindings[i].stageFlags = vkStage;
	m_vVkBindings[i].pImmutableSamplers = nullptr;

	vector<VkDescriptorImageInfo> vTexDescs(uNum);
	for (auto i = 0u; i < uNum; ++i)
		vTexDescs[i] = ppTextures[i]->GetDesc();
	m_vvImageDesc.push_back(vTexDescs);
	assert(m_vvImageDesc.data());

	memset(&m_vVkWrites[i], 0, sizeof(m_vVkWrites[i]));
	m_vVkWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	m_vVkWrites[i].dstBinding = m_vVkBindings[i].binding;
	m_vVkWrites[i].descriptorCount = uNum;
	m_vVkWrites[i].descriptorType = vkType;
	m_vVkWrites[i].pImageInfo = m_vvImageDesc[m_vvImageDesc.size() - 1].data();
}

void DescSet::AttachTextures(const uint8_t uNum,
	const ppTexture ppTextures,
	const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	const auto uBinding = uint32_t(m_vVkBindings.size());
	const auto vkBinding = VkDescriptorSetLayoutBinding
	{
		uBinding,	//.binding = 1,
		vkType,		//.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		uNum,		//.descriptorCount = DEMO_TEXTURE_COUNT,
		vkStage,	//.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		nullptr		//.pImmutableSamplers = NULL,
	};
	m_vVkBindings.push_back(vkBinding);
	assert(m_vVkBindings.data());

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
	m_vVkWrites.push_back(write);
	assert(m_vVkWrites.data());
}

void DescSet::AttachTextures(const uint8_t uNum,
	const ppImage ppTextures,
	const VkShaderStageFlags vkStage,
	const VkDescriptorType vkType)
{
	const auto uBinding = uint32_t(m_vVkBindings.size());
	const auto vkBinding = VkDescriptorSetLayoutBinding
	{
		uBinding,	//.binding = 1,
		vkType,		//.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		uNum,		//.descriptorCount = DEMO_TEXTURE_COUNT,
		vkStage,	//.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		nullptr		//.pImmutableSamplers = NULL,
	};
	m_vVkBindings.push_back(vkBinding);
	assert(m_vVkBindings.data());

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
	m_vVkWrites.push_back(write);
	assert(m_vVkWrites.data());
}

const VkDescriptorSet &DescSet::Get() const
{
	return m_VkDescSet;
}

const VkDescriptorSetLayout &DescSet::GetLayout() const
{
	return m_VkDescLayout;
}
