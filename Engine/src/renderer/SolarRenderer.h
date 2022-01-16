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

		static bool8 LoadAllModels();
		//static bool8 LoadModel(ModelResource model);

		static bool8 LoadAllTextures();
		static bool8 LoadAllPrograms();


		static void Shutdown();
		static void* GetNativeDeviceContext();
	};
}
