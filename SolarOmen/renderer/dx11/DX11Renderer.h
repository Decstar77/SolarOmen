#pragma once
#include "DX11Header.h"
#include "DX11Commands.h"
#include "DX11Shader.h"
#include "DX11StaticMesh.h"
#include "DX11Texture.h"
#include "DX11Techniques.h"

namespace cm
{
	struct RenderDebug
	{
		uint64 next;
		struct IDXGIInfoQueue* info_queue;

		ShaderInstance shader;
		ID3D11Buffer* vertex_buffer;
	};

	struct SwapChain
	{
		IDXGISwapChain* swapChain;
		ID3D11DepthStencilView* depthView;
		ID3D11ShaderResourceView* depthShaderView;
		ID3D11RenderTargetView* renderView;
	};

#define GetRenderState() RenderState* rs = RenderState::Get()

	struct RenderState
	{
		ID3D11Device* device;
		ID3D11DeviceContext* context;

		SwapChain swapChain;

		// @NOTE: Useful meshes for rendering
		StaticMesh quad;
		StaticMesh cube;

		ID3D11RasterizerState* rasterBackFaceCullingState;
		ID3D11RasterizerState* rasterFrontFaceCullingState;
		ID3D11RasterizerState* rasterNoFaceCullState;

		ID3D11DepthStencilState* depthLessState;
		ID3D11DepthStencilState* depthOffState;
		ID3D11DepthStencilState* depthLessEqualState;

		ID3D11BlendState* blendNormal;

		SamplerInstance pointRepeat;
		SamplerInstance bilinearRepeat;
		SamplerInstance trilinearRepeat;
		SamplerInstance shadowPFC;

		ShaderConstBuffer<ShaderConstBufferModel> modelConstBuffer;
		ShaderConstBuffer<ShaderConstBufferView> viewConstBuffer;
		ShaderConstBuffer<ShaderConstBufferLightingInfo> lightingConstBuffer;
		ShaderConstBuffer<ShaderConstBufferUIData> uiConstBuffer;

		ShaderInstance unlitShader;
		ShaderInstance phongShader;
		ShaderInstance phongKenneyShader;
		ShaderInstance textShader;
		ShaderInstance quadShader;
		ShaderInstance skyboxShader;
		ShaderInstance eqiToCubeShader;
		ShaderInstance irradianceConvolutionShader;

		CubeMapInstance skyboxMap;
		CubeMapInstance environmentMap;

		HashMap<StaticMesh> meshes;
		HashMap<TextureInstance> textures;

		FixedArray<TextureInstance, 128> fontTextures;
		StaticMesh fontMesh;

		RenderDebug debug;

		inline static RenderState* Get() { return renderState; }
		inline static void Initialize(RenderState* rs) { renderState = rs; }
	private:
		inline static RenderState* renderState = nullptr;
	};

	inline void LogDirectXDebugGetMessages(RenderDebug* debug)
	{
		uint64 end = debug->info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (uint64 i = debug->next; i < end; i++)
		{
			SIZE_T messageLength = 0;
			debug->info_queue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength);

			auto bytes = std::make_unique<byte[]>(messageLength);
			auto message = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());

			debug->info_queue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength);

			LOG(message->pDescription);
		}
	}

	inline void InitializeDirectXDebugLogging()
	{
		GetRenderState();

		HRESULT dxresult;
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		HMODULE mod_debug = LoadLibraryExA("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (mod_debug)
		{
			DXGIGetDebugInterface debug_fnc = reinterpret_cast<DXGIGetDebugInterface>(
				reinterpret_cast<void*>(GetProcAddress(mod_debug, "DXGIGetDebugInterface")));
			if (debug_fnc)
			{
				dxresult = debug_fnc(__uuidof(IDXGIInfoQueue), (void**)&rs->debug.info_queue);
			}
			else
			{
				//TODO: Loggin
			}
		}
		else
		{
			//TODO: Loggin
		}
	}

	inline DXGI_FORMAT GetTextureFormatToD3D(const TextureFormat& format)
	{
		switch (format)
		{
		case TextureFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
		case TextureFormat::R8_BYTE: return DXGI_FORMAT_R8_UINT;
		case TextureFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
		case TextureFormat::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
		case TextureFormat::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
		case TextureFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
		case TextureFormat::R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
		case TextureFormat::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
		case TextureFormat::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
		case TextureFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TextureFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	inline uint32 GetTextureFormatElementSizeBytes(const TextureFormat& format)
	{
		switch (format)
		{
		case TextureFormat::R8G8B8A8_UNORM: return sizeof(uint8);
		case TextureFormat::R16G16_UNORM: return sizeof(uint16);
		case TextureFormat::R8_BYTE: return sizeof(uint8);
		case TextureFormat::R32_FLOAT: return sizeof(real32);
		case TextureFormat::D32_FLOAT: return sizeof(real32);
		case TextureFormat::R32_TYPELESS: return sizeof(real32);
		case TextureFormat::R16_UNORM: return sizeof(uint16);
		case TextureFormat::D16_UNORM: return sizeof(uint16);
		case TextureFormat::R16_TYPELESS: return sizeof(uint16);
		case TextureFormat::R32G32_FLOAT: return sizeof(real32);
		case TextureFormat::R32G32B32_FLOAT: return sizeof(real32);
		case TextureFormat::R32G32B32A32_FLOAT: return sizeof(real32);
		case TextureFormat::R16G16B16A16_FLOAT: return sizeof(uint16);
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return 0;
	}

	inline uint32 GetTextureFormatElementCount(const TextureFormat& format)
	{
		switch (format)
		{
		case TextureFormat::R8G8B8A8_UNORM: return 4;
		case TextureFormat::R16G16_UNORM: return 2;
		case TextureFormat::R8_BYTE: return 1;
		case TextureFormat::R32_FLOAT: return 1;
		case TextureFormat::D32_FLOAT: return 1;
		case TextureFormat::R32_TYPELESS: return 1;
		case TextureFormat::R16_UNORM: return 1;
		case TextureFormat::D16_UNORM: return 1;
		case TextureFormat::R16_TYPELESS: return 1;
		case TextureFormat::R32G32_FLOAT: return 2;
		case TextureFormat::R32G32B32_FLOAT: return 3;
		case TextureFormat::R32G32B32A32_FLOAT: return 4;
		case TextureFormat::R16G16B16A16_FLOAT: return 4;
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return 0;
	}

	inline D3D11_TEXTURE_ADDRESS_MODE GetTextureWrapModeToD3D(const TextureWrapMode& wrap)
	{
		switch (wrap)
		{
		case TextureWrapMode::REPEAT: return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureWrapMode::CLAMP_EDGE:return D3D11_TEXTURE_ADDRESS_CLAMP;
		default: Assert(0, "TextureWrapModeToD3D ??");
		}

		return D3D11_TEXTURE_ADDRESS_WRAP;
	}

	inline D3D11_FILTER GetTextureFilterModeToD3D(const TextureFilterMode& mode)
	{
		switch (mode)
		{
		case TextureFilterMode::POINT:		return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case TextureFilterMode::BILINEAR:	return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TextureFilterMode::TRILINEAR:	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		default: Assert(0, "TextureFilterModeToD3D ??");
		}

		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	inline int32 GetTextureUsageToD3DBindFlags(const TextureUsage& usage)
	{
		switch (usage)
		{
		case TextureUsage::NONE:  return 0;
		case TextureUsage::SHADER_RESOURCE: return D3D11_BIND_SHADER_RESOURCE;
		case TextureUsage::RENDER_TARGET: return D3D11_BIND_RENDER_TARGET;
		case TextureUsage::DEPTH_SCENCIL_BUFFER: return D3D11_BIND_DEPTH_STENCIL;
		case TextureUsage::COMPUTER_SHADER_RESOURCE: return D3D11_BIND_UNORDERED_ACCESS;
		default: Assert(0, "TextureUsageToD3DBindFlags ??");
		}

		return 0;
	}

	inline int32 GetTextureCPUFlagsToD3DFlags(const TextureCPUFlags& flags)
	{
		switch (flags)
		{
		case TextureCPUFlags::NONE: return 0;
		case TextureCPUFlags::READ: return D3D11_CPU_ACCESS_READ;
		case TextureCPUFlags::WRITE: return D3D11_CPU_ACCESS_WRITE;
		case TextureCPUFlags::READ_WRITE: return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		}
		return 0;
	}
}
