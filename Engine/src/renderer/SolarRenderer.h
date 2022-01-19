#pragma once

#include "RendererTypes.h"

namespace sol
{
#define USE_DIRECTX11 1
#define USE_DIRECX12 0

	class SOL_API Renderer
	{
	public:
		static bool8 LoadAllModels();

		//static bool8 LoadModel(ModelResource model);

		static TextureHandle CreateTexture(int32 width, int32 height, TextureFormat format, ResourceCPUFlags cpuFlags);
		static void UpdateWholeTexture(const TextureHandle& handle, void* pixelData);
		static void DestroyTexture(TextureHandle* handle);

		static bool8 LoadAllTextures();

		static bool8 LoadAllPrograms();
		static void* GetNativeDeviceContext();

	private:
		static bool8 Initialize();
		static void Render(RenderPacket* renderPacket);
		static void Shutdown();

		friend class Application;
	};
}
