#pragma once

#include "RendererTypes.h"

namespace sol
{
#define USE_DIRECX11 0
#define USE_DIRECX12 1

	class SOL_API Renderer
	{
	public:
		static bool8 Initialize();
		static void Render(RenderPacket* renderPacket);
		static void Shutdown();
		static void* GetNativeDeviceContext();
	};
}
