#pragma once

#include "RendererTypes.h"

namespace sol
{
	class Renderer
	{
	public:
		static bool8 Initialize();
		static void Render(RenderPacket* renderPacket);
		static void Shutdown();
	};
}
