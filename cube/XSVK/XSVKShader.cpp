//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKShader.h"

using namespace std;
using namespace XSVK;

Shader::Shader(const VkDevice pVkDevice) :
	m_pVkDevice(pVkDevice)
{
	memset(m_pVertexShaders, 0, sizeof(VkPipelineShaderStageCreateInfo[MAX_SHADER_NUM]));
	memset(m_pFragmentShaders, 0, sizeof(VkPipelineShaderStageCreateInfo[MAX_SHADER_NUM]));
}

Shader::~Shader()
{
	Reset();
}

void Shader::CreateVertexShader(const uint8_t i,
	const pchar szFilename, const pchar szMain)
{
	m_pVertexShaders[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_pVertexShaders[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
	m_pVertexShaders[i].module = createShader(szFilename);
	m_pVertexShaders[i].pName = szMain;
}

void Shader::CreateFragmentShader(const uint8_t i,
	const pchar szFilename, const pchar szMain)
{
	m_pFragmentShaders[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_pFragmentShaders[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	m_pFragmentShaders[i].module = createShader(szFilename);
	m_pFragmentShaders[i].pName = szMain;
}

void Shader::Reset()
{
	for (auto &shader : m_pVertexShaders)
		if (shader.module)
		{
			vkDestroyShaderModule(m_pVkDevice, shader.module, nullptr);
			shader.module = VK_NULL_HANDLE;
		}
	for (auto &shader : m_pFragmentShaders)
		if (shader.module)
		{
			vkDestroyShaderModule(m_pVkDevice, shader.module, nullptr);
			shader.module = VK_NULL_HANDLE;
		}
}

const VkPipelineShaderStageCreateInfo &Shader::GetVertexShader(const uint8_t i) const
{
	return m_pVertexShaders[i];
}

const VkPipelineShaderStageCreateInfo &Shader::GetFragmentShader(const uint8_t i) const
{
	return m_pFragmentShaders[i];
}

upuints Shader::readSPV(const pchar szFilename, size_t &uSize)
{
	//long int size;
	size_t U_ASSERT_ONLY retval;

	FILE *fp = fopen(szFilename, "rb");
	if (!fp) return nullptr;

	fseek(fp, 0L, SEEK_END);
	uSize = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	auto shader_code = make_unique<uint32_t[]>(static_cast<size_t>(ceil(uSize / 4.0f)));
	assert(shader_code);
	retval = fread(shader_code.get(), uSize, 1, fp);
	assert(retval == 1);

	fclose(fp);

	return shader_code;
}

VkShaderModule Shader::createShader(const pchar szFilename)
{
	size_t uSize;

	const auto pShaderCode = readSPV(szFilename, uSize);

	return createShader(pShaderCode.get(), uSize);
}

VkShaderModule Shader::createShader(const puint uCode, const size_t uSize)
{
	VkShaderModule module;
	VkShaderModuleCreateInfo moduleCreateInfo;

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = nullptr;

	moduleCreateInfo.codeSize = uSize;
	moduleCreateInfo.pCode = uCode;
	moduleCreateInfo.flags = 0;
	VkResult U_ASSERT_ONLY err = vkCreateShaderModule(m_pVkDevice,
		&moduleCreateInfo, nullptr, &module);
	assert(!err);

	return module;
}
