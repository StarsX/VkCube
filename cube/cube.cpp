//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "Demo.h"

using namespace std;

//--------------------------------------------------------------------------------------
// Mesh and VertexFormat Data
//--------------------------------------------------------------------------------------
// clang-format off
/*
struct Vertex
{
	float     posX, posY, posZ, posW;    // Position data
	float     r, g, b, a;                // Color
};

struct VertexPosTex
{
	float     posX, posY, posZ, posW;    // Position data
	float     u, v, s, t;                // Texcoord
};

#define XYZ1(_x_, _y_, _z_)         (_x_), (_y_), (_z_), 1.f
#define UV(_u_, _v_)                (_u_), (_v_), 0.f, 1.f
*/

void dumpMatrix(const char *note, mat4x4 MVP)
{
	int i;

	printf("%s: \n", note);
	for (i = 0; i < 4; i++) {
		printf("%f, %f, %f, %f\n", MVP[i][0], MVP[i][1], MVP[i][2], MVP[i][3]);
	}
	printf("\n");
	fflush(stdout);
}

void dumpVec4(const char *note, vec4 vector)
{
	printf("%s: \n", note);
	printf("%f, %f, %f, %f\n", vector[0], vector[1], vector[2], vector[3]);
	printf("\n");
	fflush(stdout);
}

// On MS-Windows, make this a global, so it's available to WndProc()
unique_ptr<Demo> g_pDemo;

void OnCreate()
{
	g_pDemo->Create();
}

void OnFrameMove()
{
	g_pDemo->Update();
}

void OnFrameRender()
{
	g_pDemo->Render();
}

void OnDestroy()
{
	g_pDemo->Destroy();

#ifndef _WIN32
	xcb_destroy_window(demo->connection, demo->window);
	xcb_disconnect(demo->connection);
	free(demo->atom_wm_delete_window);
#endif // _WIN32
}

void OnResize(const uint32_t uWidth, const uint32_t uHeight)
{
	g_pDemo->Resize(uWidth, uHeight);
}

#ifdef _WIN32
// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		XSVK::Close();
		break;
	case WM_PAINT:
		XSVK::Run();
		break;
	case WM_SIZE:
		// Resize the application to the new window size, except when
		// it was minimized. Vulkan doesn't support images or swapchains
		// with width=0 and height=0.
		if (wParam != SIZE_MINIMIZED)
		{
			uint32_t uWidth = lParam & 0xffff;
			uint32_t uHeight = (lParam & 0xffff0000) >> 16;
			XSVK::Resize(uWidth, uHeight);
		}
		break;
	default:
		break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}
#else  // _WIN32
static void handleEvent(struct demo *demo,
	const xcb_generic_event_t *event)
{
	uint8_t event_code = event->response_type & 0x7f;
	switch (event_code)
	{
	case XCB_EXPOSE:
		// TODO: Resize window
		break;
	case XCB_CLIENT_MESSAGE:
		if ((*(xcb_client_message_event_t *)event).data.data32[0] ==
			(*demo->atom_wm_delete_window).atom)
			demo->quit = true;
		break;
	case XCB_KEY_RELEASE:
	{
		const xcb_key_release_event_t *key =
			(const xcb_key_release_event_t *)event;

		switch (key->detail)
		{
		case 0x9: // Escape
			demo->quit = true;
			break;
		case 0x71: // left arrow key
			demo->spin_angle += demo->spin_increment;
			break;
		case 0x72: // right arrow key
			demo->spin_angle -= demo->spin_increment;
			break;
		case 0x41:
			demo->pause = !demo->pause;
			break;
		}
	}
	break;
	case XCB_CONFIGURE_NOTIFY:
	{
		const xcb_configure_notify_event_t *cfg =
			(const xcb_configure_notify_event_t *)event;
		if ((demo->width != cfg->width) || (demo->height != cfg->height))
		{
			demo->width = cfg->width;
			demo->height = cfg->height;
			demo_resize(demo);
		}
	}
	break;
	default:
		break;
	}
}
#endif // _WIN32

#ifdef _WIN32
// Include header required for parsing the command line options.
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	int argc;
	std::vector<std::vector<char>> argv;

	// Use the CommandLine functions to get the command line arguments.
	// Unfortunately, Microsoft outputs
	// this information as wide characters for Unicode, and we simply want the
	// Ascii version to be compatible
	// with the non-Windows side.  So, we have to convert the information to
	// Ascii character strings.
	LPWSTR *commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (nullptr == commandLineArgs) argc = 0;

	if (argc > 0)
	{
		argv.resize(argc);
		assert(argv.data());
		if (argv.size() == 0) argc = 0;
		else
		{
			for (int iii = 0; iii < argc; iii++)
			{
				size_t wideCharLen = wcslen(commandLineArgs[iii]);
				size_t numConverted = 0;

				argv[iii].resize(wideCharLen + 1);
				if (argv.size() > 0)
					wcstombs_s(&numConverted, argv[iii].data(), wideCharLen + 1,
						commandLineArgs[iii], wideCharLen + 1);
			}
		}
	}
	//else argv = nullptr;

	g_pDemo = make_unique<Demo>();
	g_pDemo->Init(XSVK::CreateContext(argc, argv.data()));

	XSVK::InitWindow("Vulkan sample", hInstance, 500u, 500u);
	XSVK::SetCreateCallback(OnCreate);
	XSVK::SetFrameMoveCallback(OnFrameMove);
	XSVK::SetFrameRenderCallback(OnFrameRender);
	XSVK::SetDestroyCallback(OnDestroy);
	XSVK::SetResizeCallback(OnResize);

	return XSVK::Loop();
}
#else  // _WIN32
int main(int argc, char **argv)
{
	struct demo demo;

	Init(&demo, argc, argv);
	CreateAppWindow(&demo);
	initVkSwapchain(&demo);

	OnCreate(&demo);
	Run(&demo);

	OnDestroy(&demo);

	return validation_error;
}
#endif // _WIN32
