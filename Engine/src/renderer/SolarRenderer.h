#pragma once

#include "RendererTypes.h"

namespace sol
{
	class SOL_API Renderer
	{
	public:
		static bool8 Initialize();
		static void Render(RenderPacket* renderPacket);
		static void Shutdown();

		static void* GetNativeDeviceContext();
	};
}
