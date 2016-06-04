//--------------------------------------------------------------------------------------
// Revised by S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVKCommon.h"
#include "XSVKContext.h"
#include "XSVKContextMacros.h"

using namespace std;
using namespace XSVK;

int Context::m_iValidError = 0;

Context::Context() :
	m_VkCmdPool(VK_NULL_HANDLE)
{
}

Context::~Context()
{
	Reset();
}

void Context::Init(int argc, vector<char>* argv)
{
	m_iFrameCount = INT32_MAX;

	for (int i = 1; i < argc; ++i)
	{
		const auto argvi = argv[i].data();
		if (strcmp(argvi, "--use_staging") == 0)
		{
			m_bUseStaging = true;
			continue;
		}
		if (strcmp(argvi, "--break") == 0)
		{
			m_bUseBreak = true;
			continue;
		}
		if (strcmp(argvi, "--validate") == 0)
		{
			m_bValidate = true;
			continue;
		}
		if (strcmp(argvi, "--c") == 0 && m_iFrameCount == INT32_MAX &&
			i < argc - 1 && sscanf(argv[i + 1].data(), "%d", &m_iFrameCount) == 1 &&
			m_iFrameCount >= 0)
		{
			++i;
			continue;
		}

		fprintf(stderr, "Usage:\n  Vulkan [--use_staging] [--validate] [--break] "
						"[--c <framecount>]\n");
		fflush(stderr);
		exit(1);
	}

	//initConnection(demo);
	initVk();
}

#ifdef _WIN32
void Context::InitWindow(const pchar szName, HINSTANCE hConnection,
	uint32_t uWidth, uint32_t uHeight)
{
	strcpy(m_szName, szName);
	m_hConnection = hConnection;
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	WNDCLASSEX win_class;

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WndProc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = m_hConnection; // hInstance
	win_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = nullptr;
	win_class.lpszClassName = m_szName;
	win_class.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassEx(&win_class))
	{
		// It didn't work, so try to give a useful error:
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}
	// Create window with the registered class:
	RECT wr = { 0, 0, (LONG)m_uWidth, (LONG)m_uHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	m_hWindow = CreateWindowEx(0,
		m_szName,					// class name
		m_szName,					// app name
		WS_OVERLAPPEDWINDOW |		// window style
		WS_VISIBLE | WS_SYSMENU,
		100, 100,					// x/y coords
		wr.right - wr.left,			// width
		wr.bottom - wr.top,			// height
		nullptr,					// handle to parent
		nullptr,					// handle to menu
		m_hConnection,				// hInstance
		nullptr);					// no extra parameters
	if (!m_hWindow)
	{
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}
}
#else  // _WIN32
static void CreateAppWindow(struct demo *demo) {
	uint32_t value_mask, value_list[32];

	demo->window = xcb_generate_id(demo->connection);

	value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	value_list[0] = demo->screen->black_pixel;
	value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY;

	xcb_create_window(demo->connection, XCB_COPY_FROM_PARENT, demo->window,
		demo->screen->root, 0, 0, demo->width, demo->height, 0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT, demo->screen->root_visual,
		value_mask, value_list);

	/* Magic code that will send notification when window is destroyed */
	xcb_intern_atom_cookie_t cookie =
		xcb_intern_atom(demo->connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t *reply =
		xcb_intern_atom_reply(demo->connection, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 =
		xcb_intern_atom(demo->connection, 0, 16, "WM_DELETE_WINDOW");
	demo->atom_wm_delete_window =
		xcb_intern_atom_reply(demo->connection, cookie2, 0);

	xcb_change_property(demo->connection, XCB_PROP_MODE_REPLACE, demo->window,
		(*reply).atom, 4, 32, 1,
		&(*demo->atom_wm_delete_window).atom);
	free(reply);

	xcb_map_window(demo->connection, demo->window);

	// Force the x/y coordinates to 100,100 results are identical in consecutive
	// runs
	const uint32_t coords[] = { 100, 100 };
	xcb_configure_window(demo->connection, demo->window,
		XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
}
#endif // _WIN32

void Context::InitVkSwapchain()
{
	VkResult U_ASSERT_ONLY err;

// Create a WSI surface for the window:
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0u;
	createInfo.hinstance = m_hConnection;
	createInfo.hwnd = m_hWindow;

	err = vkCreateWin32SurfaceKHR(m_pVkInst, &createInfo, nullptr, &m_VkSurface);

#else  // _WIN32
	VkXcbSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.connection = demo->connection;
	createInfo.window = demo->window;

	err = vkCreateXcbSurfaceKHR(demo->inst, &createInfo, nullptr, &demo->surface);
#endif // _WIN32

	// Iterate over each queue to learn whether it supports presenting:
	const auto uNumQueue = m_vVkQueueProps.size();
	auto vSupportsPresent = vector<VkBool32>(uNumQueue);
	for (auto i = 0u; i < uNumQueue; ++i)
		m_FxGetPhysicalDeviceSurfaceSupportKHR(m_pVkGPU, i, m_VkSurface,
			&vSupportsPresent[i]);

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (auto i = 0u; i < uNumQueue; ++i)
	{
		if ((m_vVkQueueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == UINT32_MAX)
			{
				graphicsQueueNodeIndex = i;
			}

			if (vSupportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (auto i = 0u; i < uNumQueue; ++i)
		{
			if (vSupportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Generate error if could not find both a graphics and a present queue
	if (graphicsQueueNodeIndex == UINT32_MAX ||
		presentQueueNodeIndex == UINT32_MAX)
		ERR_EXIT("Could not find a graphics and a present queue\n",
				 "Swapchain Initialization Failure");

	// TODO: Add support for separate queues, including presentation,
	//       synchronization, and appropriate tracking for QueueSubmit.
	// NOTE: While it is possible for an application to use a separate graphics
	//       and a present queues, this demo program assumes it is only using
	//       one:
	if (graphicsQueueNodeIndex != presentQueueNodeIndex)
		ERR_EXIT("Could not find a common graphics and a present queue\n",
				 "Swapchain Initialization Failure");

	m_uGQueueNodeIdx = graphicsQueueNodeIndex;

	createDevice();

	GET_DEVICE_PROC_ADDR(m_pVkDevice, CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_pVkDevice, DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_pVkDevice, GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(m_pVkDevice, AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(m_pVkDevice, QueuePresentKHR);

	vkGetDeviceQueue(m_pVkDevice, m_uGQueueNodeIdx, 0u, &m_pVkQueue);

	// Get the list of VkFormat's that are supported:
	uint32_t uNumformat;
	err = m_FxGetPhysicalDeviceSurfaceFormatsKHR(m_pVkGPU, m_VkSurface,
		&uNumformat, nullptr);
	assert(!err);
	auto vSurfFormats = vector<VkSurfaceFormatKHR>(uNumformat);
	err = m_FxGetPhysicalDeviceSurfaceFormatsKHR(m_pVkGPU, m_VkSurface,
		&uNumformat, vSurfFormats.data());
	assert(!err);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (uNumformat == 1 && vSurfFormats[0].format == VK_FORMAT_UNDEFINED)
		m_VkFormat = VK_FORMAT_B8G8R8A8_UNORM;
	else
	{
		assert(uNumformat >= 1);
		m_VkFormat = vSurfFormats[0].format;
	}
	m_VkColorSpace = vSurfFormats[0].colorSpace;

	m_bQuit = false;
	m_iCurFrame = 0;

	// Get Memory information and properties
	vkGetPhysicalDeviceMemoryProperties(m_pVkGPU, &m_VkMemoryProps);
}

void Context::Create()
{
	m_FxResize(m_uWidth, m_uHeight);

	createCommandPool();

	createSwapchain();
	createBackBuffers();

	m_FxCreate();

	for (const auto &pBuffer : m_vpBuffers)
		pBuffer->AllocCommandBuffer(m_VkCmdPool);

	m_uCurBuffer = 0u;
	m_bPrepared = true;
}

void Context::Destroy()
{
	// Don't react to resize until after first initialization.
	if (!m_bPrepared)  return;
	// In order to properly resize the window, we must re-create the swapchain
	// AND redo the command buffers, etc.
	//
	// First, perform part of the demo_cleanup() function:
	m_bPrepared = false;

	m_vpBuffers.clear();
	m_FxDestroy();
}

void Context::Resize(const uint32_t uWidth, const uint32_t uHeight)
{
	// Don't react to resize until after first initialization.
	if (!m_bPrepared) return;

	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_FxResize(uWidth, uHeight);
	
	Destroy();
	destroyCommandPool();

	// Second, re-perform the demo_prepare() function, which will re-create the
	// swapchain:
	Create();
}

void Context::SetCreateCallback(const FxVoid &fxCreate)
{
	m_FxCreate = fxCreate;
}

void Context::SetFrameMoveCallback(const FxVoid &fxFrameMove)
{
	m_FxFrameMove = fxFrameMove;
}

void Context::SetFrameRenderCallback(const FxVoid &fxFrameRender)
{
	m_FxFrameRender = fxFrameRender;
}

void Context::SetDestroyCallback(const FxVoid &fxDestroy)
{
	m_FxDestroy = fxDestroy;
}

void Context::SetResizeCallback(const FxDim &fxResize)
{
	m_FxResize = fxResize;
}

#ifdef _WIN32
void Context::Run()
{
	if (!m_bPrepared) return;

	// Wait for work to finish before updating MVP.
	vkDeviceWaitIdle(m_pVkDevice);

	m_FxFrameMove();
	render();
	present();

	// Wait for work to finish before updating MVP.
	vkDeviceWaitIdle(m_pVkDevice);

	++m_iCurFrame;
	if (m_iFrameCount != INT_MAX && m_iCurFrame == m_iFrameCount)
		PostQuitMessage(m_iValidError);
}
#else
static void Run(struct demo *demo) {
	xcb_flush(demo->connection);

	while (!demo->quit) {
		xcb_generic_event_t *event;

		if (demo->pause) {
			event = xcb_wait_for_event(demo->connection);
		}
		else {
			event = xcb_poll_for_event(demo->connection);
		}
		if (event) {
			handleEvent(demo, event);
			free(event);
		}

		// Wait for work to finish before updating MVP.
		vkDeviceWaitIdle(demo->device);
		demo_update_data_buffer(demo);

		demo_draw(demo);

		// Wait for work to finish before updating MVP.
		vkDeviceWaitIdle(demo->device);
		demo->curFrame++;
		if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount)
			demo->quit = true;
	}
}
#endif

void Context::Reset()
{
	if (m_pVkDevice && m_VkSwapchain)
		m_FxDestroySwapchainKHR(m_pVkDevice, m_VkSwapchain, nullptr);

	destroyCommandPool();
	if (m_pVkDevice) vkDestroyDevice(m_pVkDevice, nullptr);
	if (m_bValidate)
		m_VkDestroyDebugReportCallback(m_pVkInst, m_VkMsgCallback, nullptr);

	if (m_pVkInst && m_VkSurface)
		vkDestroySurfaceKHR(m_pVkInst, m_VkSurface, nullptr);
	if (m_pVkInst) vkDestroyInstance(m_pVkInst, nullptr);
}

const HWND &Context::GetWindow() const
{
	return m_hWindow;
}

const VkPhysicalDevice &Context::GetGPU() const
{
	return m_pVkGPU;
}

const VkDevice &Context::GetDevice() const
{
	return m_pVkDevice;
}

const VkQueue &Context::GetQueue() const
{
	return m_pVkQueue;
}

const VkCommandPool &Context::GetCmdPool() const
{
	return m_VkCmdPool;
}

const VkFormat &Context::GetFormat() const
{
	return m_VkFormat;
}

const VkPhysicalDeviceMemoryProperties &Context::GetMemProps() const
{
	return m_VkMemoryProps;
}

const VkCommandBuffer &Context::GetCurCmd() const
{
	return m_vpBuffers[m_uCurBuffer]->GetCmd();
}

const vpImage &Context::GetBuffers() const
{
	return m_vpBuffers;
}

uint8_t Context::GetCurBuffer() const
{
	return m_uCurBuffer;
}

bool Context::IsUseStaging() const
{
	return m_bUseStaging;
}

int Context::GetValidationError()
{
	return m_iValidError;
}

void Context::initVk()
{
	VkResult err;
	uint32_t instance_extension_count = 0u;
	uint32_t instance_layer_count = 0u;
	uint32_t device_validation_layer_count = 0u;
	ppchar instance_validation_layers = nullptr;
	m_uNumEnabledExtension = 0u;
	m_uNumEnabledLayer = 0u;

	pchar instance_validation_layers_alt1[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	pchar instance_validation_layers_alt2[] =
	{
		"VK_LAYER_GOOGLE_threading",     "VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_image",         "VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_swapchain",     "VK_LAYER_GOOGLE_unique_objects"
	};

	/* Look for validation layers */
	VkBool32 validation_found = VK_FALSE;
	if (m_bValidate)
	{

		err = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
		assert(!err);

		instance_validation_layers = instance_validation_layers_alt1;
		if (instance_layer_count > 0u)
		{
			auto instance_layers = vector<VkLayerProperties>(instance_layer_count);
			err = vkEnumerateInstanceLayerProperties(&instance_layer_count,
					instance_layers.data());
			assert(!err);


			validation_found = checkLayers(
					ARRAY_SIZE(instance_validation_layers_alt1),
					instance_validation_layers, instance_layer_count,
					instance_layers.data());
			if (validation_found)
			{
				m_uNumEnabledLayer = ARRAY_SIZE(instance_validation_layers_alt1);
				m_pSzDevValidLayers[0] = "VK_LAYER_LUNARG_standard_validation";
				device_validation_layer_count = 1u;
			}
			else
			{
				// use alternative set of validation layers
				instance_validation_layers = instance_validation_layers_alt2;
				m_uNumEnabledLayer = ARRAY_SIZE(instance_validation_layers_alt2);
				validation_found = checkLayers(
					ARRAY_SIZE(instance_validation_layers_alt2),
					instance_validation_layers, instance_layer_count,
					instance_layers.data());
				device_validation_layer_count =
						ARRAY_SIZE(instance_validation_layers_alt2);
				for (uint32_t i = 0; i < device_validation_layer_count; i++)
					m_pSzDevValidLayers[i] =
							instance_validation_layers[i];
			}
		}

		if (!validation_found)
			ERR_EXIT("vkEnumerateInstanceLayerProperties failed to find"
					"required validation layer.\n\n"
					"Please look at the Getting Started guide for additional "
					"information.\n",
					"vkCreateInstance Failure");
	}

	/* Look for instance extensions */
	VkBool32 surfaceExtFound = VK_FALSE;
	VkBool32 platformSurfaceExtFound = VK_FALSE;
	memset(m_pSzExtNames, 0, sizeof(m_pSzExtNames));

	err = vkEnumerateInstanceExtensionProperties(
		nullptr, &instance_extension_count, nullptr);
	assert(!err);

	if (instance_extension_count > 0u)
	{
		auto instance_extensions = vector<VkExtensionProperties>(instance_extension_count);
		err = vkEnumerateInstanceExtensionProperties(
			nullptr, &instance_extension_count, instance_extensions.data());
		assert(!err);
		for (auto i = 0u; i < instance_extension_count; ++i)
		{
			if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME,
						instance_extensions[i].extensionName))
			{
				surfaceExtFound = VK_TRUE;
				m_pSzExtNames[m_uNumEnabledExtension++] =
					VK_KHR_SURFACE_EXTENSION_NAME;
			}
#ifdef _WIN32
			if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
						instance_extensions[i].extensionName))
			{
				platformSurfaceExtFound = VK_TRUE;
				m_pSzExtNames[m_uNumEnabledExtension++] =
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			}
#else  // _WIN32
			if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME,
						instance_extensions[i].extensionName))
			{
				platformSurfaceExtFound = VK_TRUE;
				m_pSzExtNames[m_uNumEnabledExtension++] =
					VK_KHR_XCB_SURFACE_EXTENSION_NAME;
			}
#endif // _WIN32
			if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
						instance_extensions[i].extensionName))
				if (m_bValidate)
					m_pSzExtNames[m_uNumEnabledExtension++] =
						VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			assert(m_uNumEnabledExtension < 64u);
		}
	}

	if (!surfaceExtFound)
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
				 "the " VK_KHR_SURFACE_EXTENSION_NAME
				 " extension.\n\nDo you have a compatible "
				 "Vulkan installable client driver (ICD) installed?\nPlease "
				 "look at the Getting Started guide for additional "
				 "information.\n",
				 "vkCreateInstance Failure");
	if (!platformSurfaceExtFound)
	{
#ifdef _WIN32
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
				 "the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME
				 " extension.\n\nDo you have a compatible "
				 "Vulkan installable client driver (ICD) installed?\nPlease "
				 "look at the Getting Started guide for additional "
				 "information.\n",
				 "vkCreateInstance Failure");
#else  // _WIN32
		ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find "
				 "the " VK_KHR_XCB_SURFACE_EXTENSION_NAME
				 " extension.\n\nDo you have a compatible "
				 "Vulkan installable client driver (ICD) installed?\nPlease "
				 "look at the Getting Started guide for additional "
				 "information.\n",
				 "vkCreateInstance Failure");
#endif // _WIN32
	}
	const VkApplicationInfo app =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,				//.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,										//.pNext = NULL,
		m_szName,										//.pApplicationName = APP_SHORT_NAME,
		0u,												//.applicationVersion = 0,
		m_szName,										//.pEngineName = APP_SHORT_NAME,
		0u,												//.engineVersion = 0,
		VK_API_VERSION_1_0,								//.apiVersion = VK_API_VERSION_1_0,
	};
	VkInstanceCreateInfo inst_info =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,				//.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,											//.pNext = NULL,
		0u,													//.flag = 0,
		&app,												//.pApplicationInfo = &app,
		m_uNumEnabledLayer,									//.enabledLayerCount = demo->enabled_layer_count,
		(const char *const *)instance_validation_layers,	//.ppEnabledLayerNames = (const char *const *)instance_validation_layers,
		m_uNumEnabledExtension,								//.enabledExtensionCount = demo->enabled_extension_count,
		(const char *const *)m_pSzExtNames					//.ppEnabledExtensionNames = (const char *const *)demo->extension_names,
	};

	/*
	 * This is info for a temp callback to use during CreateInstance.
	 * After the instance is created, we use the instance-based
	 * function to register the final callback.
	 */
	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
	if (m_bValidate)
	{
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.pNext = nullptr;
		dbgCreateInfo.pfnCallback = m_bUseBreak ? breakCallback : dbgFunc;
		dbgCreateInfo.pUserData = nullptr;
		dbgCreateInfo.flags =
			VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		inst_info.pNext = &dbgCreateInfo;
	}

	uint32_t gpu_count;

	err = vkCreateInstance(&inst_info, nullptr, &m_pVkInst);
	if (err == VK_ERROR_INCOMPATIBLE_DRIVER)
		ERR_EXIT("Cannot find a compatible Vulkan installable client driver "
				 "(ICD).\n\nPlease look at the Getting Started guide for "
				 "additional information.\n",
				 "vkCreateInstance Failure");
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT)
		ERR_EXIT("Cannot find a specified extension library"
				 ".\nMake sure your layers path is set appropriately.\n",
				 "vkCreateInstance Failure");
	else if (err)
		ERR_EXIT("vkCreateInstance failed.\n\nDo you have a compatible Vulkan "
				 "installable client driver (ICD) installed?\nPlease look at "
				 "the Getting Started guide for additional information.\n",
				 "vkCreateInstance Failure");

	/* Make initial call to query gpu_count, then second call for gpu info*/
	err = vkEnumeratePhysicalDevices(m_pVkInst, &gpu_count, nullptr);
	assert(!err && gpu_count > 0u);

	if (gpu_count > 0u)
	{
		auto physical_devices = vector<VkPhysicalDevice>(gpu_count);
		err = vkEnumeratePhysicalDevices(m_pVkInst, &gpu_count, physical_devices.data());
		assert(!err);
		/* For cube demo we just grab the first physical device */
		m_pVkGPU = physical_devices[0];
	}
	else ERR_EXIT("vkEnumeratePhysicalDevices reported zero accessible devices.\n\n"
				 "Do you have a compatible Vulkan installable client driver (ICD) "
				 "installed?\nPlease look at the Getting Started guide for "
				 "additional information.\n",
				 "vkEnumeratePhysicalDevices Failure");

	/* Look for validation layers */
	validation_found = VK_FALSE;
	m_uNumEnabledLayer = 0u;
	auto device_layer_count = 0u;
	err = vkEnumerateDeviceLayerProperties(m_pVkGPU, &device_layer_count, nullptr);
	assert(!err);

	if (device_layer_count > 0u)
	{
		auto device_layers = vector<VkLayerProperties>(device_layer_count);
		err = vkEnumerateDeviceLayerProperties(m_pVkGPU, &device_layer_count,
			device_layers.data());
		assert(!err);

		if (m_bValidate)
		{
			validation_found = checkLayers(device_validation_layer_count,
				m_pSzDevValidLayers,
				device_layer_count,
				device_layers.data());
			m_uNumEnabledLayer = device_validation_layer_count;
		}
	}

	if (m_bValidate && !validation_found)
		ERR_EXIT("vkEnumerateDeviceLayerProperties failed to find"
				 "a required validation layer.\n\n"
				 "Please look at the Getting Started guide for additional "
				 "information.\n",
				 "vkCreateDevice Failure");

	/* Look for device extensions */
	uint32_t device_extension_count = 0u;
	VkBool32 swapchainExtFound = VK_FALSE;
	m_uNumEnabledExtension = 0u;
	memset(m_pSzExtNames, 0, sizeof(m_pSzExtNames));

	err = vkEnumerateDeviceExtensionProperties(m_pVkGPU, nullptr,
		&device_extension_count, nullptr);
	assert(!err);

	if (device_extension_count > 0u)
	{
		auto device_extensions = vector<VkExtensionProperties>(device_extension_count);
		err = vkEnumerateDeviceExtensionProperties(
			m_pVkGPU, nullptr, &device_extension_count, device_extensions.data());
		assert(!err);

		for (auto i = 0u; i < device_extension_count; ++i)
		{
			if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
						device_extensions[i].extensionName))
			{
				swapchainExtFound = VK_TRUE;
				m_pSzExtNames[m_uNumEnabledExtension++] =
					VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			}
			assert(m_uNumEnabledExtension < 64u);
		}
	}

	if (!swapchainExtFound)
		ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find "
				 "the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
				 " extension.\n\nDo you have a compatible "
				 "Vulkan installable client driver (ICD) installed?\nPlease "
				 "look at the Getting Started guide for additional "
				 "information.\n",
				 "vkCreateInstance Failure");

	if (m_bValidate)
	{
		m_VkCreateDebugReportCallback =
			(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
				m_pVkInst, "vkCreateDebugReportCallbackEXT");
		m_VkDestroyDebugReportCallback =
			(PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
				m_pVkInst, "vkDestroyDebugReportCallbackEXT");
		if (!m_VkCreateDebugReportCallback)
			ERR_EXIT(
				"GetProcAddr: Unable to find vkCreateDebugReportCallbackEXT\n",
				"vkGetProcAddr Failure");
		if (!m_VkDestroyDebugReportCallback)
			ERR_EXIT(
				"GetProcAddr: Unable to find vkDestroyDebugReportCallbackEXT\n",
				"vkGetProcAddr Failure");
		m_VkDebugReportMessage =
			(PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(
				m_pVkInst, "vkDebugReportMessageEXT");
		if (!m_VkDebugReportMessage) {
			ERR_EXIT("GetProcAddr: Unable to find vkDebugReportMessageEXT\n",
					 "vkGetProcAddr Failure");
		}

		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
		PFN_vkDebugReportCallbackEXT callback;
		callback = m_bUseBreak ? breakCallback : dbgFunc;
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.pNext = nullptr;
		dbgCreateInfo.pfnCallback = callback;
		dbgCreateInfo.pUserData = nullptr;
		dbgCreateInfo.flags =
			VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		err = m_VkCreateDebugReportCallback(m_pVkInst, &dbgCreateInfo, nullptr,
			&m_VkMsgCallback);
		switch (err)
		{
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			ERR_EXIT("CreateDebugReportCallback: out of host memory\n",
					 "CreateDebugReportCallback Failure");
			break;
		default:
			ERR_EXIT("CreateDebugReportCallback: unknown failure\n",
					 "CreateDebugReportCallback Failure");
			break;
		}
	}
	vkGetPhysicalDeviceProperties(m_pVkGPU, &m_VkGPUProps);

	/* Call with NULL data to get count */
	uint32_t uNumQueue;
	vkGetPhysicalDeviceQueueFamilyProperties(m_pVkGPU, &uNumQueue, nullptr);
	assert(uNumQueue >= 1u);

	m_vVkQueueProps.resize(uNumQueue);
	m_vVkQueueProps.shrink_to_fit();
	assert(m_vVkQueueProps.data());
	vkGetPhysicalDeviceQueueFamilyProperties(m_pVkGPU, &uNumQueue,
		m_vVkQueueProps.data());
	// Find a queue that supports gfx
	auto gfx_queue_idx = 0u;
	for (; gfx_queue_idx < uNumQueue; ++gfx_queue_idx)
		if (m_vVkQueueProps[gfx_queue_idx].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			break;
	assert(gfx_queue_idx < uNumQueue);
	// Query fine-grained feature support for this device.
	//  If app has specific feature requirements it should check supported
	//  features based on this query
	VkPhysicalDeviceFeatures physDevFeatures;
	vkGetPhysicalDeviceFeatures(m_pVkGPU, &physDevFeatures);

	GET_INSTANCE_PROC_ADDR(m_pVkInst, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(m_pVkInst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(m_pVkInst, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(m_pVkInst, GetPhysicalDeviceSurfacePresentModesKHR);
}

void Context::createDevice()
{
	VkResult U_ASSERT_ONLY err;
	float queue_priorities[1] = { 0.0f };
	const VkDeviceQueueCreateInfo queue =
	{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,	//.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr,									//.pNext = NULL,
		0u,											//.flags = 0,
		m_uGQueueNodeIdx,							//.queueFamilyIndex = demo->graphics_queue_node_index,
		1u,											//.queueCount = 1,
		queue_priorities							//.pQueuePriorities = queue_priorities
	};

	VkDeviceCreateInfo device = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,		//.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,									//.pNext = NULL,
		0u,											//.flags 0,
		1u,											//.queueCreateInfoCount = 1,
		&queue,										//.pQueueCreateInfos = &queue,
		m_uNumEnabledLayer,							//.enabledLayerCount = demo->enabled_layer_count,
		(const char *const *)((m_bValidate)
		? m_pSzDevValidLayers : nullptr),
		//.ppEnabledLayerNames =
		//	(const char *const *)((demo->validate)
		//							  ? demo->device_validation_layers
		//							  : NULL),
		m_uNumEnabledExtension,						//.enabledExtensionCount = demo->enabled_extension_count,
		(const char *const *)m_pSzExtNames,			//.ppEnabledExtensionNames = (const char *const *)demo->extension_names,
		nullptr,									//.pEnabledFeatures = NULL, // If specific features are required, pass them in here
	};

	err = vkCreateDevice(m_pVkGPU, &device, nullptr, &m_pVkDevice);
	assert(!err);
}

void Context::createSwapchain()
{
	VkSwapchainKHR oldVkSwapchain = m_VkSwapchain;

	// Check the surface capabilities and formats
	VkSurfaceCapabilitiesKHR surfCapabilities;
	VkResult U_ASSERT_ONLY err = m_FxGetPhysicalDeviceSurfaceCapabilitiesKHR(
		m_pVkGPU, m_VkSurface, &surfCapabilities);
	assert(!err);

	uint32_t uNumPresentMode;
	err = m_FxGetPhysicalDeviceSurfacePresentModesKHR(
		m_pVkGPU, m_VkSurface, &uNumPresentMode, nullptr);
	assert(!err);
	auto vPresentModes = vector<VkPresentModeKHR>(uNumPresentMode);
	assert(vPresentModes.data());
	err = m_FxGetPhysicalDeviceSurfacePresentModesKHR(
		m_pVkGPU, m_VkSurface, &uNumPresentMode, vPresentModes.data());
	assert(!err);

	VkExtent2D swapchainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = m_uWidth;
		swapchainExtent.height = m_uHeight;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
		m_uWidth = surfCapabilities.currentExtent.width;
		m_uHeight = surfCapabilities.currentExtent.height;
	}

	// If mailbox mode is available, use it, as is the lowest-latency non-
	// tearing mode.  If not, try IMMEDIATE which will usually be available,
	// and is fastest (though it tears).  If not, fall back to FIFO which is
	// always available.
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto &presentMode : vPresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
			(presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR))
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	auto desiredNumberOfSwapchainImages =
		surfCapabilities.minImageCount + 1u;
	if ((surfCapabilities.maxImageCount > 0u) &&
		(desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount))
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else preTransform = surfCapabilities.currentTransform;

	const VkSwapchainCreateInfoKHR swapchain =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,		//.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,											//.pNext = NULL,
		0u,													//.flags = 0,
		m_VkSurface,										//.surface = demo->surface,											
		desiredNumberOfSwapchainImages,						//.minImageCount = desiredNumberOfSwapchainImages,
		m_VkFormat,											//.imageFormat = demo->format,
		m_VkColorSpace,										//.imageColorSpace = demo->color_space,
		{ swapchainExtent.width, swapchainExtent.height },	//.imageExtent = {.width = swapchainExtent.width, .height = swapchainExtent.height},
		1u,													//.imageArrayLayers = 1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,				//.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,							//.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		0u,													//.queueFamilyIndexCount = 0,
		nullptr,											//.pQueueFamilyIndices = NULL,
		preTransform,										//.preTransform = preTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,					//.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		swapchainPresentMode,								//.presentMode = swapchainPresentMode,
		true,												//.clipped = true,
		oldVkSwapchain										//.oldSwapchain =
	};

	err = m_FxCreateSwapchainKHR(m_pVkDevice, &swapchain, nullptr, &m_VkSwapchain);
	assert(!err);

	// If we just re-created an existing swapchain, we should destroy the old
	// swapchain at this point.
	// Note: destroying the swapchain also cleans up all its associated
	// presentable images once the platform is done with them.
	if (oldVkSwapchain != VK_NULL_HANDLE)
		m_FxDestroySwapchainKHR(m_pVkDevice, oldVkSwapchain, nullptr);
}

void Context::createBackBuffers()
{
	uint32_t uNumSwapchainImage;
	VkResult U_ASSERT_ONLY err = m_FxGetSwapchainImagesKHR(
		m_pVkDevice, m_VkSwapchain, &uNumSwapchainImage, nullptr);
	assert(!err);

	auto vSwapchainImages = vector<VkImage>(uNumSwapchainImage);
	assert(vSwapchainImages.data());
	err = m_FxGetSwapchainImagesKHR(m_pVkDevice, m_VkSwapchain,
		&uNumSwapchainImage, vSwapchainImages.data());
	assert(!err);

	m_vpBuffers.resize(uNumSwapchainImage);
	m_vpBuffers.shrink_to_fit();
	assert(m_vpBuffers.data());

	for (auto i = 0ui8; i < uNumSwapchainImage; ++i)
	{
		m_vpBuffers[i] = make_unique<ImageBuffer>(
			m_pVkDevice, m_VkMemoryProps);
		assert(m_vpBuffers[i]);

		m_vpBuffers[i]->SetFormat(m_VkFormat);
		m_vpBuffers[i]->Create(vSwapchainImages[i]);
	}
}

void Context::createCommandPool()
{
	const VkCommandPoolCreateInfo cmdPoolInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,		//.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,										//.pNext = NULL,
		0u,												//.flags = 0,
		m_uGQueueNodeIdx,								//.queueFamilyIndex = demo->graphics_queue_node_index,
	};

	VkResult U_ASSERT_ONLY err = vkCreateCommandPool(m_pVkDevice, &cmdPoolInfo, nullptr,
		&m_VkCmdPool);
	assert(!err);
}

void Context::destroyCommandPool()
{
	if (m_pVkDevice && m_VkCmdPool)
	{
		vkDestroyCommandPool(m_pVkDevice, m_VkCmdPool, nullptr);
		m_VkCmdPool = VK_NULL_HANDLE;
	}
}

void Context::render()
{
	static const VkCommandBufferInheritanceInfo cmdBufHInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,	//.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		nullptr,											//.pNext = NULL,
		VK_NULL_HANDLE,										//.renderPass = VK_NULL_HANDLE,
		0u,													//.subpass = 0,
		VK_NULL_HANDLE,										//.framebuffer = VK_NULL_HANDLE,
		VK_FALSE,											//.occlusionQueryEnable = VK_FALSE,
		0x00000000,											//.queryFlags = 0,
		0x00000000											//.pipelineStatistics = 0,
	};
	static const VkCommandBufferBeginInfo cmdBufInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,		//.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,											//.pNext = NULL,
		0x00000000,											//.flags = 0,
		&cmdBufHInfo										//.pInheritanceInfo = &cmd_buf_hinfo,
	};

	const auto uNumSwapchainImage = uint8_t(m_vpBuffers.size());
	for (auto i = 0ui8; i < uNumSwapchainImage; ++i)
	{
		m_uCurBuffer = i;
		const auto &pVkCmd = m_vpBuffers[i]->GetCmd();

		VkResult U_ASSERT_ONLY err =
			vkBeginCommandBuffer(pVkCmd, &cmdBufInfo);
		assert(!err);

		// Assume the command buffer has been run on current_buffer before so
		// we need to set the image layout back to COLOR_ATTACHMENT_OPTIMAL
		m_vpBuffers[i]->SetBarrier(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		m_FxFrameRender();

		m_vpBuffers[i]->SetBarrier(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		err = vkEndCommandBuffer(pVkCmd);
		assert(!err);
	}
	
}

void Context::present()
{
	VkResult U_ASSERT_ONLY err;
	VkSemaphore presentCompleteSemaphore;
	const VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo =
	{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,	//.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,									//.pNext = NULL,
		0u											//.flags = 0,
	};
	VkFence nullFence = VK_NULL_HANDLE;

	err = vkCreateSemaphore(m_pVkDevice, &presentCompleteSemaphoreCreateInfo,
		nullptr, &presentCompleteSemaphore);
	assert(!err);

	// Get the index of the next available swapchain image:
	err = m_FxAcquireNextImageKHR(m_pVkDevice, m_VkSwapchain, UINT64_MAX,
		presentCompleteSemaphore, (VkFence)0, // TODO: Show use of fence
		&m_uCurBuffer);
	if (err == VK_ERROR_OUT_OF_DATE_KHR)
	{
		// demo->swapchain is out of date (e.g. the window was resized) and
		// must be recreated:
		Resize(m_uWidth, m_uHeight);
		present();
		vkDestroySemaphore(m_pVkDevice, presentCompleteSemaphore, nullptr);
		return;
	}
	else if (err == VK_SUBOPTIMAL_KHR)
	{
		// demo->swapchain is not as optimal as it could be, but the platform's
		// presentation engine will still present the image correctly.
	}
	else assert(!err);

	// Wait for the present complete semaphore to be signaled to ensure
	// that the image won't be rendered to until the presentation
	// engine has fully released ownership to the application, and it is
	// okay to render to the image.

	// FIXME/TODO: DEAL WITH VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	const VkPipelineStageFlags pipe_stage_flags =
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	const VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,			//.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,								//.pNext = NULL,
		1u,										//.waitSemaphoreCount = 1,
		&presentCompleteSemaphore,				//.pWaitSemaphores = &presentCompleteSemaphore,
		&pipe_stage_flags,						//.pWaitDstStageMask = &pipe_stage_flags,
		1u,										//.commandBufferCount = 1,
		&m_vpBuffers[m_uCurBuffer]->GetCmd(),	//.pCommandBuffers = &demo->buffers[demo->current_buffer].cmd,
		0u,										//.signalSemaphoreCount = 0,
		nullptr									//.pSignalSemaphores = NULL
	};

	err = vkQueueSubmit(m_pVkQueue, 1u, &submitInfo, nullFence);
	assert(!err);

	const VkPresentInfoKHR present =
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,	//.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,							//.pNext = NULL,
		0u,									//.waitSemaphoreCount = 0,
		nullptr,							//.pWaitSemaphores = NULL,
		1u,									//.swapchainCount = 1,
		&m_VkSwapchain,						//.pSwapchains = &demo->swapchain,
		&m_uCurBuffer						//.pImageIndices = &demo->current_buffer,
	};

	// TBD/TODO: SHOULD THE "present" PARAMETER BE "const" IN THE HEADER?
	err = m_FxQueuePresentKHR(m_pVkQueue, &present);
	if (err == VK_ERROR_OUT_OF_DATE_KHR)
		// demo->swapchain is out of date (e.g. the window was resized) and
		// must be recreated:
		Resize(m_uWidth, m_uHeight);
	else if (err == VK_SUBOPTIMAL_KHR)
	{
		// demo->swapchain is not as optimal as it could be, but the platform's
		// presentation engine will still present the image correctly.
	}
	else assert(!err);

	err = vkQueueWaitIdle(m_pVkQueue);
	assert(err == VK_SUCCESS);

	vkDestroySemaphore(m_pVkDevice, presentCompleteSemaphore, nullptr);
}

/*
* Return 1 (true) if all layer names specified in check_names
* can be found in given layer properties.
*/
VkBool32 Context::checkLayers(const uint32_t uNumCheck,
	const ppchar pSzCheckNames, const uint32_t uNumlayer,
	const pVkLayerProperties pVklayers)
{
	for (auto i = 0u; i < uNumCheck; ++i)
	{
		auto found = VK_FALSE;
		for (uint32_t j = 0; j < uNumlayer; ++j)
		{
			if (!strcmp(pSzCheckNames[i], pVklayers[j].layerName))
			{
				found = VK_TRUE;
				break;
			}
		}
		if (!found)
		{
			fprintf(stderr, "Cannot find layer: %s\n", pSzCheckNames[i]);
			return 0;
		}
	}
	return 1;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
Context::dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject, size_t location, int32_t msgCode,
	const pchar pLayerPrefix, const pchar pMsg, const pvoid pUserData)
{
	auto message = vector<char>(strlen(pMsg) + 100);

	assert(message.data());

	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		sprintf(message.data(), "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode,
			pMsg);
		Context::m_iValidError = 1;
	}
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		// We know that we're submitting queues without fences, ignore this
		// warning
		if (strstr(pMsg,
			"vkQueueSubmit parameter, VkFence fence, is null pointer"))
			return false;

		sprintf(message.data(), "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode,
			pMsg);
		Context::m_iValidError = 1;
	}
	else
	{
		Context::m_iValidError = 1;
		return false;
	}

#ifdef _WIN32
	MessageBox(nullptr, message.data(), "Alert", MB_OK);
#else
	printf("%s\n", message);
	fflush(stdout);
#endif

	/*
	* false indicates that layer should not bail-out of an
	* API call that had validation failures. This may mean that the
	* app dies inside the driver due to invalid parameter(s).
	* That's what would happen without validation layers, so we'll
	* keep that behavior here.
	*/
	return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
Context::breakCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject, size_t location, int32_t msgCode,
	const pchar pLayerPrefix, const pchar pMsg,
	const pvoid pUserData)
{
#ifndef WIN32
	raise(SIGTRAP);
#else
	DebugBreak();
#endif

	return false;
}
