//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#define APP_NAME_STR_LEN 80
#endif // _WIN32

#include "XSVKPipeline.h"
#include "XSVKIABuffer.h"
#include "XSVKContext.h"
#include "linmath.h"

namespace XSVK
{
	using FxVoid = std::function<void()>;
	using FxDim = std::function<void(const uint32_t, const uint32_t)>;

	void Init(int argc, std::vector<char> *argv);
	void InitWindow(char *szName, HINSTANCE hConnection,
		const uint32_t uWidth, const uint32_t uHeight);
	void SetCreateCallback(const FxVoid &fxCreate);
	void SetFrameMoveCallback(const FxVoid &fxFrameMove);
	void SetFrameRenderCallback(const FxVoid &fxFrameRender);
	void SetDestroyCallback(const FxVoid &fxDestroy);
	void SetResizeCallback(const FxDim &fxResize);
	void Run();
	void Resize(const uint32_t uWidth, const uint32_t uHeight);
	void Close();

	int Loop();

	Context *CreateContext(int argc, std::vector<char> *argv);
	Context *GetContext();
}
