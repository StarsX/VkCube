//--------------------------------------------------------------------------------------
// Revised by S. XU Tianchen
//--------------------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <functional>
#include <vulkan/vulkan.h>
#include "XSVKFramebuffer.h"
//#include <vulkan/vk_sdk_platform.h>

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

namespace XSVK
{
	using upImage = std::unique_ptr<ImageBuffer>;
	using vpImage = std::vector<upImage>;

	class Context
	{
	public:
		using pchar = std::add_pointer_t<const char>;
		using FxVoid = std::function<void()>;
		using FxDim = std::function<void(const uint32_t, const uint32_t)>;
		using vVkImage = std::vector<VkImage>;

		Context();
		virtual ~Context();

		void Init(int argc, std::vector<char>*argv);
#ifdef _WIN32
		void InitWindow(const pchar szName, HINSTANCE hConnection,
			const uint32_t uWidth, const uint32_t uHeight);
#endif
		void InitVkSwapchain();
		void Create();
		void Destroy();
		void Resize(const uint32_t uWidth, const uint32_t uHeight);

		void SetCreateCallback(const FxVoid &fxCreate);
		void SetFrameMoveCallback(const FxVoid &fxFrameMove);
		void SetFrameRenderCallback(const FxVoid &fxFrameRender);
		void SetDestroyCallback(const FxVoid &fxDestroy);
		void SetResizeCallback(const FxDim &fxResize);

#ifdef _WIN32
		void Run();
#endif
		void Reset();

#ifdef _WIN32
		const HWND										&GetWindow()	const;
#endif
		const VkPhysicalDevice							&GetGPU()		const;
		const VkDevice									&GetDevice()	const;
		const VkQueue									&GetQueue()		const;
		const VkCommandPool								&GetCmdPool()	const;
		const VkFormat									&GetFormat()	const;
		const VkPhysicalDeviceMemoryProperties			&GetMemProps()	const;
		const VkCommandBuffer							&GetCurCmd()	const;

		const vpImage									&GetBuffers()	const;
		uint8_t											GetCurBuffer()	const;

		bool IsUseStaging() const;

		static int										GetValidationError();

	protected:
		using upFramebuffer = std::unique_ptr<Framebuffer>;
		using vpFramebuffer = std::vector<upFramebuffer>;
		using vVkQueueFamilyProperties = std::vector<VkQueueFamilyProperties>;
		using pVkLayerProperties = std::add_pointer_t<VkLayerProperties>;
		using ppchar = std::add_pointer_t<pchar>;
		using pvoid = std::add_pointer_t<void>;

		void initVk();
		void createDevice();
		void createSwapchain();
		void createBackBuffers();
		void createCommandPool();
		void destroyCommandPool();
		void render();
		void present();

#ifdef _WIN32
#define APP_NAME_STR_LEN 80
		HINSTANCE										m_hConnection;				// hInstance - Windows Instance
		char											m_szName[APP_NAME_STR_LEN];	// Name to put on the window/icon
		HWND											m_hWindow;					// hWnd - window handle
#else																				// _WIN32
		xcb_connection_t								*connection;
		xcb_screen_t									*screen;
		xcb_window_t									window;
		xcb_intern_atom_reply_t							*atom_wm_delete_window;
#endif																				// _WIN32
		VkSurfaceKHR									m_VkSurface;

		VkInstance										m_pVkInst;
		VkPhysicalDevice								m_pVkGPU;

		VkDevice										m_pVkDevice;
		VkQueue											m_pVkQueue;

		VkPhysicalDeviceProperties						m_VkGPUProps;
		VkPhysicalDeviceMemoryProperties				m_VkMemoryProps;
		vVkQueueFamilyProperties						m_vVkQueueProps;

		vpImage											m_vpBuffers;
		vpFramebuffer									m_vpFramebuffers;

		uint32_t										m_uGQueueNodeIdx;

		uint32_t											m_uCurBuffer;
		uint32_t										m_uWidth;
		uint32_t										m_uHeight;

		uint32_t										m_uNumEnabledExtension;
		uint32_t										m_uNumEnabledLayer;
		pchar											m_pSzExtNames[64];
		pchar											m_pSzDevValidLayers[64];

		VkFormat										m_VkFormat;
		VkColorSpaceKHR									m_VkColorSpace;

		PFN_vkGetPhysicalDeviceSurfaceSupportKHR		m_FxGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	m_FxGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		m_FxGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	m_FxGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkCreateSwapchainKHR						m_FxCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR						m_FxDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR						m_FxGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR						m_FxAcquireNextImageKHR;
		PFN_vkQueuePresentKHR							m_FxQueuePresentKHR;

		VkSwapchainKHR									m_VkSwapchain;
		VkCommandPool									m_VkCmdPool;

		int32_t											m_iCurFrame;
		int32_t											m_iFrameCount;
		bool											m_bQuit;
		bool											m_bValidate;
		bool											m_bUseBreak;
		bool											m_bUseStaging;
		bool											m_bPrepared;

		PFN_vkCreateDebugReportCallbackEXT				m_VkCreateDebugReportCallback;
		PFN_vkDestroyDebugReportCallbackEXT				m_VkDestroyDebugReportCallback;
		PFN_vkDebugReportMessageEXT						m_VkDebugReportMessage;
		VkDebugReportCallbackEXT						m_VkMsgCallback;

		FxVoid											m_FxCreate;
		FxVoid											m_FxFrameMove;
		FxVoid											m_FxFrameRender;
		FxVoid											m_FxDestroy;
		FxDim											m_FxResize;

		static VkBool32 checkLayers(const uint32_t uNumCheck,
			const ppchar pSzCheckNames, const uint32_t uNumlayer,
			const pVkLayerProperties pVklayers);
		static VKAPI_ATTR VkBool32 VKAPI_CALL
			dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
				uint64_t srcObject, size_t location, int32_t msgCode,
				const pchar pLayerPrefix, const pchar pMsg, const pvoid pUserData);
		static VKAPI_ATTR VkBool32 VKAPI_CALL
			breakCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
				uint64_t srcObject, size_t location, int32_t msgCode,
				const pchar pLayerPrefix, const pchar pMsg,
				const pvoid pUserData);
		static int m_iValidError;
	};
}
