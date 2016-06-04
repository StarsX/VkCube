//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#include "XSVK.h"

using namespace std;
using namespace XSVK;

unique_ptr<Context> g_pContext;

void XSVK::Init(int argc, std::vector<char> *argv)
{
	g_pContext = make_unique<Context>();
	g_pContext->Init(argc, argv);
}

void XSVK::InitWindow(char *szName, HINSTANCE hConnection,
	const uint32_t uWidth, const uint32_t uHeight)
{
	g_pContext->InitWindow(szName, hConnection, uWidth, uHeight);
}

void XSVK::SetCreateCallback(const FxVoid &fxCreate)
{
	g_pContext->SetCreateCallback(fxCreate);
}

void XSVK::SetFrameMoveCallback(const FxVoid &fxFrameMove)
{
	g_pContext->SetFrameMoveCallback(fxFrameMove);
}

void XSVK::SetFrameRenderCallback(const FxVoid &fxFrameRender)
{
	g_pContext->SetFrameRenderCallback(fxFrameRender);
}

void XSVK::SetDestroyCallback(const FxVoid &fxDestroy)
{
	g_pContext->SetDestroyCallback(fxDestroy);
}

void XSVK::SetResizeCallback(const FxDim &fxResize)
{
	g_pContext->SetResizeCallback(fxResize);
}

void XSVK::Run()
{
	g_pContext->Run();
}

void XSVK::Resize(const uint32_t uWidth, const uint32_t uHeight)
{
	g_pContext->Resize(uWidth, uHeight);
}

void XSVK::Close()
{
	PostQuitMessage(Context::GetValidationError());
}

int XSVK::Loop()
{
	MSG msg;   // message
	bool done; // flag saying when app is complete

	g_pContext->InitVkSwapchain();
	g_pContext->Create();

	done = false; // initialize loop condition variable

				  // main message loop
	while (!done)
	{
		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (msg.message == WM_QUIT) // check for a quit message
			done = true; // if found, quit app
		else
		{
			/* Translate and dispatch to event queue*/
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RedrawWindow(g_pContext->GetWindow(), nullptr, nullptr, RDW_INTERNALPAINT);
	}

	g_pContext->Destroy();

	return (int)msg.wParam;
}

Context *XSVK::CreateContext(int argc, std::vector<char> *argv)
{
	Init(argc, argv);
	
	return g_pContext.get();
}

Context *XSVK::GetContext()
{
	return g_pContext.get();
}
