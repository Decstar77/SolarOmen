#pragma once

#include "RendererTypes.h"

namespace sol
{
#define USE_DIRECTX11 1
#define USE_DIRECX12 0

	class SOL_API Renderer
	{
	public:
		static bool8 Initialize();
		static void Render(RenderPacket* renderPacket);
		static void Shutdown();
		static void* GetNativeDeviceContext();
	};
}
