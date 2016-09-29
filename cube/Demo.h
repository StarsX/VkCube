#pragma once

#include "XSVK.h"

class Demo
{
public:
	enum ShaderPass
	{
		BASE_PASS,
		SHADE_PASS,
		NUM_PASS
	};

	struct vktexcube_vb
	{
		float position[3];
		float normal[3];
		float tex[2];
	};

	Demo();
	virtual ~Demo();

	void Init(XSVK::Context *pContext);
	void Resize(const uint32_t uWidth, const uint32_t uHeight);

	void Create();
	void Update();
	void Render();
	void Destroy();

protected:
	void createBackBuffers();
	void createColorBuffers();
	void createDepthBuffer();
	void createObjectBuffers();
	void createSamplers();
	void createTextures();
	void createUniformBuffers();
	void createVertexBuffers();
	void createShaders();
	void createPipelines();
	void createFramebuffers();

	bool m_bPause;

	uint32_t m_uWidth;
	uint32_t m_uHeight;

	std::unique_ptr<XSVK::ImageBuffer>					m_ppGBuffers[2];

	std::vector<std::unique_ptr<XSVK::Texture>>			m_vpTextures;
	std::unique_ptr<XSVK::Buffer>						m_pUMatrices;
	std::unique_ptr<XSVK::Buffer>						m_pULight;
	std::unique_ptr<XSVK::IABuffer>						m_pVBCube;
	std::unique_ptr<XSVK::IABuffer>						m_pIBCube;
	std::unique_ptr<XSVK::ImageBuffer>					m_pDepth;
	std::unique_ptr<XSVK::ImageBuffer>					m_pSampler;

	std::unique_ptr<XSVK::Shader>						m_pShader;
	std::unique_ptr<XSVK::DescSet>						m_ppDescSets[NUM_PASS];
	std::unique_ptr<XSVK::RenderPass>					m_ppRenderPasses[NUM_PASS];
	std::unique_ptr<XSVK::Pipeline>						m_ppPipelines[NUM_PASS];

	std::vector<std::unique_ptr<XSVK::Framebuffer>>		m_vpFramebuffers;
	std::unique_ptr<XSVK::Framebuffer>					m_pGBuffer;

	XSVK::Context										*m_pContext;

	mat4x4												m_mProj;
	mat4x4												m_mView;
	mat4x4												m_mWorld;

	//float												m_fSpinAngle;
	//float												spin_increment;
};
