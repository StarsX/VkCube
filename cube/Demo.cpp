//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "Demo.h"

#define DEMO_TEXTURE_COUNT 1

#if defined(NDEBUG) && defined(__GNUC__)
#define U_ASSERT_ONLY __attribute__((unused))
#else
#define U_ASSERT_ONLY
#endif

#define XORSWAP(a, b)   ((&(a) == &(b)) ? (a) : ((a)^=(b),(b)^=(a),(a)^=(b)))

using namespace std;
using namespace XSVK;

static char *g_txFiles[] = { "lunarg.pfm" };

static const float g_vertex_buffer_data[] =
{
	-1.0f,-1.0f,-1.0f,  // -X side
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,

	-1.0f,-1.0f,-1.0f,  // -Z side
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,

	-1.0f,-1.0f,-1.0f,  // -Y side
	1.0f,-1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,

	-1.0f, 1.0f,-1.0f,  // +Y side
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,

	1.0f, 1.0f,-1.0f,  // +X side
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,

	-1.0f, 1.0f, 1.0f,  // +Z side
	-1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
};

static const float g_normal_buffer_data[] =
{
	-1.0f, 0.0f, 0.0f,	// -X side
	0.0f, 0.0f,-1.0f,	// -Z side
	0.0f,-1.0f, 0.0f,	// -Y side
	0.0f, 1.0f, 0.0f,	// +Y side
	1.0f, 0.0f, 0.0f,	// +X side
	0.0f, 0.0f, 1.0f,	// +Z side
};

static const float g_uv_buffer_data[] =
{
	0.0f, 0.0f,  // -X side
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	1.0f, 0.0f,  // -Z side
	0.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,

	1.0f, 1.0f,  // -Y side
	1.0f, 0.0f,
	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	1.0f, 1.0f,  // +Y side
	0.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,

	1.0f, 1.0f,  // +X side
	0.0f, 1.0f,
	0.0f, 0.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,

	0.0f, 1.0f,  // +Z side
	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
};
// clang-format on

Demo::Demo()
{
}

Demo::~Demo()
{
}

void Demo::Init(Context *pContext)
{
	m_pContext = pContext;

	vec3 eye = { 0.0f, 3.0f, 5.0f };
	vec3 origin = { 0, 0, 0 };
	vec3 up = { 0.0f, 1.0f, 0.0 };

	//demo->spin_increment = 0.01f;
	m_bPause = false;

	//mat4x4_perspective(demo->projection_matrix, (float)degreesToRadians(45.0f),
	//demo->width / (float)demo->height, 0.1f, 100.0f);
	mat4x4_look_at(m_mView, eye, origin, up);
	mat4x4_identity(m_mWorld);
}

void Demo::Resize(const uint32_t uWidth, const uint32_t uHeight)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	mat4x4_perspective(m_mProj, (float)degreesToRadians(45.0f),
		m_uWidth / (float)m_uHeight, 0.1f, 100.0f);
}

void Demo::Create()
{
	createBackBuffers();
	createObjectBuffers();

	createShaders();
	createPipelines();
	createFramebuffers();
}

void Demo::Update()
{
	// Matrices
	mat4x4 mModelViewProj, mViewProj, mModel, mWorldInv, mNormal;
	mat4x4_mul(mViewProj, m_mProj, m_mView);

	static auto uTime = timeGetTime();
	const auto uPrevTime = uTime;
	uTime = timeGetTime();

	const auto fSpinAngle = 0.05f * (uTime - uPrevTime);

	// Rotate 22.5 degrees around the Y axis
	mat4x4_dup(mModel, m_mWorld);
	mat4x4_rotate(m_mWorld, mModel, 0.0f, 1.0f, 0.0f,
		(float)degreesToRadians(fSpinAngle));
	mat4x4_mul(mModelViewProj, mViewProj, m_mWorld);
	mat4x4_invert(mWorldInv, m_mWorld);
	mat4x4_transpose(mNormal, mWorldInv);

	uint8_t *pData;
	const auto uMatrixSize = sizeof(mat4x4);
	m_pUMatrices->Map((void **)&pData);
	memcpy(pData, mModelViewProj, uMatrixSize);
	memcpy(pData + uMatrixSize, m_mWorld, uMatrixSize);
	memcpy(pData + uMatrixSize * 2, mNormal, uMatrixSize);
	m_pUMatrices->Unmap();
}

void Demo::Render()
{
	const auto pVkCmd = m_pContext->GetCurCmd();

	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.width = (float)m_uWidth;
	viewport.height = (float)m_uHeight;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(pVkCmd, 0u, 1u, &viewport);

	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = m_uWidth;
	scissor.extent.height = m_uHeight;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(pVkCmd, 0u, 1u, &scissor);

	// Base pass (G-buffers)
	{
		for (const auto &pGBuffer : m_ppGBuffers)
			pGBuffer->SetBarrier(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, pVkCmd);

		const VkClearValue clear_values[3] =
		{
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 1.0f, 0u }
		};

		const auto &pDescriptor = m_ppDescSets[0];
		const auto &pRenderpass = m_ppRenderPasses[0];
		const auto &pPipeline = m_ppPipelines[0];

		pRenderpass->SetFramebuffer(m_pGBuffer->Get());
		pRenderpass->SetRenderArea(m_uWidth, m_uHeight);
		pRenderpass->SetClearColors(clear_values, 3ui8);

		vkCmdBeginRenderPass(pVkCmd,
			&pRenderpass->GetBegin(),
			VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(pVkCmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pPipeline->Get());
		vkCmdBindDescriptorSets(pVkCmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pPipeline->GetLayout(), 0u, 1u,
			pDescriptor->Get(), 0u, nullptr);

		VkDeviceSize uOffsets[1] = { 0 };
		vkCmdBindVertexBuffers(pVkCmd, 0u, 1u,
			&m_pVBCube->Get(), uOffsets);
		vkCmdBindIndexBuffer(pVkCmd, m_pIBCube->Get(),
			0u, VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(pVkCmd, 12u * 3u, 1u, 0u, 0, 0u);
		vkCmdEndRenderPass(pVkCmd);

		for (const auto &pGBuffer : m_ppGBuffers)
			pGBuffer->SetBarrier(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				pVkCmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}

	// Shading pass
	{
		const VkClearValue clear_value =
		{ 0.2f, 0.2f, 0.2f, 0.2f };

		const auto &pDescriptor = m_ppDescSets[1];
		const auto &pRenderpass = m_ppRenderPasses[1];
		const auto &pPipeline = m_ppPipelines[1];

		pRenderpass->SetFramebuffer(
			m_vpFramebuffers[m_pContext->GetCurBuffer()]->Get());
		pRenderpass->SetRenderArea(m_uWidth, m_uHeight);
		pRenderpass->SetClearColors(&clear_value, 1ui8);

		vkCmdBeginRenderPass(pVkCmd,
			&pRenderpass->GetBegin(),
			VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(pVkCmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pPipeline->Get());
		vkCmdBindDescriptorSets(pVkCmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pPipeline->GetLayout(), 0u, 1u,
			pDescriptor->Get(), 0u, nullptr);

		vkCmdDraw(pVkCmd, 3u, 1u, 0u, 0u);
		vkCmdEndRenderPass(pVkCmd);
	}
}

void Demo::Destroy()
{
	m_vpFramebuffers.clear();
	m_pGBuffer.reset();
	m_ppPipelines[1].reset();
	m_ppPipelines[0].reset();
	m_ppRenderPasses[1].reset();
	m_ppRenderPasses[0].reset();
	m_ppDescSets[1].reset();
	m_ppDescSets[0].reset();
	m_pShader.reset();

	m_vpTextures.clear();

	m_pSampler.reset();
	m_pDepth.reset();
	m_ppGBuffers[1].reset();
	m_ppGBuffers[0].reset();
	m_pIBCube.reset();
	m_pVBCube.reset();
	m_pULight.reset();
	m_pUMatrices.reset();
}

void Demo::createBackBuffers()
{
	createColorBuffers();
	createDepthBuffer();
}

void Demo::createColorBuffers()
{
	for (auto &pGBuffer : m_ppGBuffers)
	{
		pGBuffer = make_unique<ImageBuffer>(
			m_pContext->GetDevice(), m_pContext->GetMemProps());
		assert(pGBuffer);
		pGBuffer->SetDimensions(m_uWidth, m_uHeight);
		pGBuffer->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT);
	}
	m_ppGBuffers[0]->SetFormat(VK_FORMAT_R8G8B8A8_UNORM);
	m_ppGBuffers[1]->SetFormat(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
	for (auto &pGBuffer : m_ppGBuffers)
		pGBuffer->Create();
		//pGBuffer->Create(VK_NULL_HANDLE, false);
}

void Demo::createDepthBuffer()
{
	m_pDepth = make_unique<ImageBuffer>(
		m_pContext->GetDevice(), m_pContext->GetMemProps());
	assert(m_pDepth);

	m_pDepth->SetFormat(VK_FORMAT_D24_UNORM_S8_UINT);
	m_pDepth->SetDimensions(m_uWidth, m_uHeight);
	m_pDepth->CreateDepth();
}

void Demo::createObjectBuffers()
{
	createSamplers();
	createTextures();
	createVertexBuffers();
	createUniformBuffers();
}

void Demo::createSamplers()
{
	m_pSampler = make_unique<ImageBuffer>(
		m_pContext->GetDevice(), m_pContext->GetMemProps());
	assert(m_pSampler);
	m_pSampler->SetUsage(VK_IMAGE_USAGE_SAMPLED_BIT);
	m_pSampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_LINEAR,
		VK_SAMPLER_MIPMAP_MODE_LINEAR);
	m_pSampler->CreateSampler();
}

// Load a ppm file into memory
bool loadTexture(const char *filename, uint8_t *rgba_data,
	VkSubresourceLayout *layout, uint32_t &width, uint32_t &height)
{
	FILE *fPtr = fopen(filename, "rb");
	char header[256], *cPtr, *tmp;

	if (!fPtr)
		return false;

	cPtr = fgets(header, 256, fPtr); // P6
	if (cPtr == nullptr || strncmp(header, "P6\n", 3))
	{
		fclose(fPtr);
		return false;
	}

	do {
		cPtr = fgets(header, 256, fPtr);
		if (cPtr == nullptr)
		{
			fclose(fPtr);
			return false;
		}
	} while (!strncmp(header, "#", 1));

	sscanf(header, "%u %u", &height, &width);
	if (rgba_data == nullptr)
	{
		fclose(fPtr);
		return true;
	}
	tmp = fgets(header, 256, fPtr); // Format
	(void)tmp;
	if (cPtr == nullptr || strncmp(header, "255\n", 3))
	{
		fclose(fPtr);
		return false;
	}

	for (auto y = 0u; y < height; ++y)
	{
		uint8_t *rowPtr = rgba_data;
		for (auto x = 0u; x < width; ++x)
		{
			size_t s = fread(rowPtr, 3, 1, fPtr);
			(void)s;
			rowPtr[3] = UINT8_MAX; // Alpha of 1
			rowPtr += 4;
		}
		rgba_data += layout->rowPitch;
	}
	fclose(fPtr);
	return true;
}

// Load a pfm file into memory
bool loadTexture(const char *filename, float *rgba_data,
	VkSubresourceLayout *layout, uint32_t &width, uint32_t &height)
{
	FILE *fPtr = fopen(filename, "rb");
	char header[256], *cPtr, *tmp;

	if (!fPtr)
		return false;

	cPtr = fgets(header, 256, fPtr); // P6
	if (cPtr == nullptr || strncmp(header, "PF\n", 3))
	{
		fclose(fPtr);
		return false;
	}

	do {
		cPtr = fgets(header, 256, fPtr);
		if (cPtr == nullptr)
		{
			fclose(fPtr);
			return false;
		}
	} while (!strncmp(header, "#", 1));

	sscanf(header, "%u %u", &width, &height);
	if (rgba_data == nullptr)
	{
		fclose(fPtr);
		return true;
	}
	tmp = fgets(header, 256, fPtr); // Format
	(void)tmp;

	float fOrder;
	sscanf(header, "%f", &fOrder);
	if (cPtr == nullptr || (fOrder != -1.0f && fOrder != 1.0f))
	{
		fclose(fPtr);
		return false;
	}

	auto byteData = (uint8_t*)rgba_data;
	if (fOrder > 0.0f) byteData += layout->rowPitch * height;
	for (auto y = 0u; y < height; ++y)
	{
		if (fOrder > 0.0f) byteData -= layout->rowPitch;
		auto rowPtr = byteData;

		for (auto x = 0u; x < width; ++x)
		{
			size_t s = fread(rowPtr, sizeof(float), 3, fPtr);
			(void)s;
			auto rgba = (float*)rowPtr;
			if (fOrder > 0.0f)
			{
				XORSWAP(rowPtr[0], rowPtr[3]);
				XORSWAP(rowPtr[1], rowPtr[2]);
				rowPtr += 4;
				XORSWAP(rowPtr[0], rowPtr[3]);
				XORSWAP(rowPtr[1], rowPtr[2]);
				rowPtr += 4;
				XORSWAP(rowPtr[0], rowPtr[3]);
				XORSWAP(rowPtr[1], rowPtr[2]);
			}
			rgba[3] = 1.0f; // Alpha of 1
			rowPtr = (uint8_t*)(rgba + 4);
		}
		if (fOrder < 0.0f) byteData += layout->rowPitch;
	}
	fclose(fPtr);

	return true;
}

void Demo::createTextures()
{
	m_vpTextures.resize(DEMO_TEXTURE_COUNT);
	m_vpTextures.shrink_to_fit();
	assert(m_vpTextures.data());
	for (auto i = 0u; i < DEMO_TEXTURE_COUNT; ++i)
	{
		const auto fileName = g_txFiles[i];
		const auto typeName = &fileName[strlen(fileName) - 4];
		const auto bHDR = strcmp(typeName, ".ppm") ? true : false;
		const VkFormat txFormat = bHDR ?
			VK_FORMAT_R32G32B32A32_SFLOAT :
			VK_FORMAT_R8G8B8A8_UNORM;

		uint32_t uTxWidth, uTxHeight;
		if (bHDR)
		{
			if (!loadTexture(fileName, (float*)nullptr, nullptr, uTxWidth, uTxHeight))
			{
				printf("Failed to load textures\n");
				fflush(stdout);
				exit(1);
			}
		}
		else
		{
			if (!loadTexture(fileName, (uint8_t*)nullptr, nullptr, uTxWidth, uTxHeight))
			{
				printf("Failed to load textures\n");
				fflush(stdout);
				exit(1);
			}
		}

		m_vpTextures[i] = make_unique<Texture>(
			m_pContext->GetDevice(), m_pContext->GetMemProps());
		assert(m_vpTextures[i]);

		m_vpTextures[i]->SetGPUQueue(m_pContext->GetGPU(), m_pContext->GetQueue());
		m_vpTextures[i]->SetFormat(txFormat);
		m_vpTextures[i]->SetDimensions(uTxWidth, uTxHeight);
		//m_vpTextures[i]->SetFilter(VK_FILTER_NEAREST, VK_FILTER_LINEAR,
		//	VK_SAMPLER_MIPMAP_MODE_LINEAR);
		m_vpTextures[i]->SetLoadFunc([fileName, bHDR](void *pData,
			VkSubresourceLayout &layout)
		{
			uint32_t tex_width, tex_height;
			if (bHDR)
			{
				if (!loadTexture(fileName, (float*)pData, &layout, tex_width, tex_height))
					fprintf(stderr, "Error loading texture: %s\n", fileName);
			}
			else
			{
				if (!loadTexture(fileName, (uint8_t*)pData, &layout, tex_width, tex_height))
					fprintf(stderr, "Error loading texture: %s\n", fileName);
			}
		});

		//m_vpTextures[i]->Create(m_pContext->GetCmdPool(), m_pContext->IsUseStaging());
		m_vpTextures[i]->Create(m_pContext->GetCmdPool(), m_pContext->IsUseStaging(), false);
	}
}

void Demo::createUniformBuffers()
{
	// Matrices
	mat4x4 mModelViewProj, mViewProj;
	mat4x4 mViewInv, mWorldInv, mNormal;

	mat4x4_mul(mViewProj, m_mProj, m_mView);
	mat4x4_mul(mModelViewProj, mViewProj, m_mWorld);
	mat4x4_invert(mViewInv, m_mView);
	mat4x4_invert(mWorldInv, m_mWorld);
	mat4x4_transpose(mNormal, mWorldInv);
	//    dumpMatrix("MVP", MVP);

	m_pUMatrices = make_unique<Buffer>(
		m_pContext->GetDevice(), m_pContext->GetMemProps());
	assert(m_pUMatrices);
	m_pUMatrices->Create(sizeof(mat4x4[3]), nullptr);

	// Light
	vec4 mLight[2] =
	{
		{ 2.0f, 4.0f, 5.0f, 0.0f },
		{ mViewInv[3][0], mViewInv[3][1], mViewInv[3][2], mViewInv[3][3] }
	};
	m_pULight = make_unique<Buffer>(
		m_pContext->GetDevice(), m_pContext->GetMemProps());
	assert(m_pULight);
	m_pULight->Create(sizeof(vec4[2]), mLight);
}

void Demo::createVertexBuffers()
{
	// Vertex buffer
	struct vktexcube_vb vb[12 * 3];
	uint16_t ib[12 * 3];
	for (auto i = 0ui8; i < 12ui8 * 3ui8; ++i) {
		vb[i].position[0] = g_vertex_buffer_data[i * 3];
		vb[i].position[1] = g_vertex_buffer_data[i * 3 + 1];
		vb[i].position[2] = g_vertex_buffer_data[i * 3 + 2];
		vb[i].normal[0] = g_normal_buffer_data[i / 6 * 3];
		vb[i].normal[1] = g_normal_buffer_data[i / 6 * 3 + 1];
		vb[i].normal[2] = g_normal_buffer_data[i / 6 * 3 + 2];
		vb[i].tex[0] = g_uv_buffer_data[2 * i];
		vb[i].tex[1] = g_uv_buffer_data[2 * i + 1];
		ib[i] = i;
	}

	m_pVBCube = make_unique<IABuffer>(
		m_pContext->GetDevice(), m_pContext->GetMemProps());
	assert(m_pVBCube);
	m_pVBCube->Create(sizeof(vb), (uint8_t*)&vb,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	m_pIBCube = make_unique<IABuffer>(
		m_pContext->GetDevice(), m_pContext->GetMemProps());
	m_pIBCube->Create(sizeof(ib), (uint8_t*)&ib,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void Demo::createShaders()
{
	m_pShader = make_unique<Shader>(m_pContext->GetDevice());
	assert(m_pShader);
	m_pShader->CreateVertexShader(0ui8, "VSBasePass.spv");
	m_pShader->CreateFragmentShader(0ui8, "FSBasePass.spv");
	m_pShader->CreateVertexShader(1ui8, "VSSSQuad.spv");
	m_pShader->CreateFragmentShader(1ui8, "FSIlluminate.spv");
}

void Demo::createPipelines()
{
	// Base pass (G-buffers)
	{
		auto &pDescSet = m_ppDescSets[0];
		pDescSet = make_unique<DescSet>(m_pContext->GetDevice());
		assert(pDescSet);
		pDescSet->AttachBuffers(0ui8, 1ui8, &m_pUMatrices,
			VK_SHADER_STAGE_VERTEX_BIT);
		pDescSet->AttachTextures(0ui8, 1ui8,
			&m_pSampler, VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_DESCRIPTOR_TYPE_SAMPLER);
		pDescSet->AttachTextures(0ui8, DEMO_TEXTURE_COUNT,
			m_vpTextures.data(), VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		//pDescSet->AttachTextures(DEMO_TEXTURE_COUNT,
		//	m_vpTextures.data(), VK_SHADER_STAGE_FRAGMENT_BIT);
		pDescSet->Create();

		auto &pRenderpass = m_ppRenderPasses[0];
		pRenderpass = make_unique<RenderPass>(m_pContext->GetDevice());
		assert(pRenderpass);
		pRenderpass->AttachColorBuffer(m_ppGBuffers[0]);
		pRenderpass->AttachColorBuffer(m_ppGBuffers[1]);
		pRenderpass->SetDepthBuffer(m_pDepth);
		pRenderpass->Create();

		auto &pPipeline = m_ppPipelines[0];
		pPipeline = make_unique<Pipeline>(m_pContext->GetDevice());
		assert(pPipeline);
		pPipeline->AttachShader(m_pShader->GetVertexShader(0ui8));
		pPipeline->AttachShader(m_pShader->GetFragmentShader(0ui8));

		pPipeline->IAAttachBindings(0ui8, sizeof(vktexcube_vb));
		pPipeline->IAAttachAttribute(0ui8, VK_FORMAT_R32G32B32_SFLOAT, 0ui8);
		pPipeline->IAAttachAttribute(0ui8, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vec3));
		pPipeline->IAAttachAttribute(0ui8, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vec3[2]));

		pPipeline->OMAttachBlend();

		pPipeline->Create(pRenderpass->Get(), pDescSet->GetLayouts());
	}

	// Shading pass
	{
		auto &pDescSet = m_ppDescSets[1];
		pDescSet = make_unique<DescSet>(m_pContext->GetDevice());
		assert(pDescSet);
		pDescSet->AttachBuffers(0ui8, 1ui8, &m_pULight,
			VK_SHADER_STAGE_FRAGMENT_BIT);
		pDescSet->AttachTextures(0ui8, 1ui8,
			&m_pSampler, VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_DESCRIPTOR_TYPE_SAMPLER);
		pDescSet->AttachTextures(0ui8, 1ui8,
			&m_ppGBuffers[0], VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		pDescSet->AttachTextures(0ui8, 1ui8,
			&m_ppGBuffers[1], VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		//pDescSet->AttachTextures(1ui8,
		//	&m_ppGBuffers[0], VK_SHADER_STAGE_FRAGMENT_BIT);
		//pDescSet->AttachTextures(1ui8,
		//	&m_ppGBuffers[1], VK_SHADER_STAGE_FRAGMENT_BIT);
		pDescSet->Create();

		auto &pRenderpass = m_ppRenderPasses[1];
		pRenderpass = make_unique<RenderPass>(m_pContext->GetDevice());
		assert(pRenderpass);
		pRenderpass->AttachColorBuffer(m_pContext->GetFormat());
		pRenderpass->Create();

		auto &pPipeline = m_ppPipelines[1];
		pPipeline = make_unique<Pipeline>(m_pContext->GetDevice());
		assert(pPipeline);
		pPipeline->AttachShader(m_pShader->GetVertexShader(1ui8));
		pPipeline->AttachShader(m_pShader->GetFragmentShader(1ui8));
		pPipeline->OMSetBlendColor(0ui8, VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
		pPipeline->OMSetBlendAlpha(0ui8, VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
		pPipeline->Create(pRenderpass->Get(), pDescSet->GetLayouts());
	}
}

void Demo::createFramebuffers()
{
	m_pGBuffer = make_unique<Framebuffer>(m_pContext->GetDevice());
	assert(m_pGBuffer);
	m_pGBuffer->SetDimensions(m_uWidth, m_uHeight);
	m_pGBuffer->SetRenderPass(m_ppRenderPasses[0]->Get());
	m_pGBuffer->SetColorBuffers(m_ppGBuffers, 2ui8);
	m_pGBuffer->SetDepthBuffer(m_pDepth);
	m_pGBuffer->Create();

	auto &vpBuffers = m_pContext->GetBuffers();
	const auto uNumSwapchainImage = uint8_t(vpBuffers.size());
	m_vpFramebuffers.resize(uNumSwapchainImage);
	m_vpFramebuffers.shrink_to_fit();
	assert(m_vpFramebuffers.data());

	for (auto i = 0ui8; i < uNumSwapchainImage; ++i)
	{
		m_vpFramebuffers[i] = make_unique<Framebuffer>(m_pContext->GetDevice());
		assert(m_vpFramebuffers[i]);
		m_vpFramebuffers[i]->SetDimensions(m_uWidth, m_uHeight);
		m_vpFramebuffers[i]->SetRenderPass(m_ppRenderPasses[1]->Get());
		m_vpFramebuffers[i]->AttachColorBuffer(vpBuffers[i]);
		m_vpFramebuffers[i]->Create();
	}
}
