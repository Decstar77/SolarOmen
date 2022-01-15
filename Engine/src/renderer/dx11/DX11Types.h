#pragma once
#include "SolarDefines.h"
#include "../SolarRenderer.h"
#include "renderer/RendererTypes.h"
#include "resources/SolarResourceTypes.h"

#include <d3d11_3.h>
#include <dxgi1_2.h>

#define DirectXDebugMessageCount 10
#if SOL_DEBUG_RENDERING
#include <dxgidebug.h>
#define DXCHECK(call)                                                                                                   \
    {                                                                                                                   \
        dc.debug.next = dc.debug.info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);                    \
        HRESULT dxresult = (call);                                                                                      \
        if (FAILED(dxresult))                                                                                           \
        {                                                                                                               \
            char *output = nullptr;                                                                                     \
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, \
                           NULL, dxresult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&output, 0, NULL);         \
            if (output)                                                                                                 \
            {                                                                                                           \
                LogDirectXDebugGetMessages(&dc.debug);															\
            }                                                                                                           \
        }                                                                                                               \
    }

#define DXINFO(call)                                         \
    {                                                        \
        dc.debug.next = dc.debug.info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL); \
		(call);                                              \
        LogDirectXDebugGetMessages(&dc.debug);		 \
    }
#else 
#define DXCHECK(call) {(call);}
#define DXINFO(call) {(call);}
#endif

//@NOTE: This is not a debug thing, it will be used in release !!!
#define DXRELEASE(object)  \
    if ((object))          \
    {                      \
        object->Release(); \
        object = nullptr;  \
    }

namespace sol
{
	struct SwapChain
	{
		IDXGISwapChain1* swapChain;
		ID3D11Texture2D* depthTexture;
		ID3D11DepthStencilView* depthView;
		ID3D11ShaderResourceView* depthShaderView;
		ID3D11RenderTargetView* renderView;
	};

#if SOL_DEBUG_RENDERING
	struct RenderDebug
	{
		uint64 next;
		struct IDXGIInfoQueue* info_queue;
		//ShaderInstance shader;
		//ID3D11Buffer* vertex_buffer;
	};

	void LogDirectXDebugGetMessages(RenderDebug* debug);
#endif

	struct DeviceContext
	{
		ID3D11Device1* device;
		ID3D11DeviceContext1* context;
#if SOL_DEBUG_RENDERING
		RenderDebug debug;
#endif
	};

	DeviceContext GetDeviceContext();

	// @TODO: Strings 
	class Topology
	{
	public:
		enum class Value
		{
			TRIANGLE_LIST,
			LINE_LIST,
		};

		Topology(Value value)
		{
			this->value = value;
		}

		inline D3D_PRIMITIVE_TOPOLOGY GetDXFormat() const
		{
			switch (value)
			{
			case Value::TRIANGLE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case Value::LINE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			}

			return {};
		}

	private:
		Value value;
	};

	struct StaticModel
	{
		uint32 strideBytes; // @NOTE: Used for rendering
		uint32 indexCount;  // @NOTE: Used for rendering
		uint32 vertexCount;  // @NOTE: Used for rendering
		VertexLayoutType vertexLayout;

		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;

		//void UpdateVertexBuffer(real32* vertices, uint32 sizeBytes);

		//static StaticModel Create(const ModelAsset& modelAsset);
		static void Release(StaticModel* mesh);
		static StaticModel Create(real32* vertices, uint32 vertexCount, VertexLayoutType layout);
		static StaticModel Create(real32* vertices, uint32 vertexCount, uint32* indices, uint32 indexCount, VertexLayoutType layout);
		static StaticModel Create(ModelResource* modelResouce);
		static StaticModel Create(const ModelResource& modelResouce);
		static StaticModel CreateScreenSpaceQuad();

	};

	struct StaticTexture
	{
		int32 width;
		int32 height;
		TextureFormat format;
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;

		ID3D11Texture2D* texture;

		ID3D11ShaderResourceView* shaderView;
		ID3D11UnorderedAccessView* uavView;
		ID3D11DepthStencilView* depthView;
		ID3D11RenderTargetView* renderView;

		static void Release(StaticTexture* texture);
		static StaticTexture Create(int32 width, int32 height, TextureFormat format, void* pixels, bool8 mips, BindUsage* usage, ResourceCPUFlags cpuFlags);
		static StaticTexture Create(TextureResource* textureResource);

		//static StaticTexture Create(const FontCharacter& fontChar);
		//static StaticTexture Create(const TextureAsset& textureAsset);
	};

	struct CubeMapInstance
	{
		ID3D11Texture2D* texture = nullptr;
		ID3D11ShaderResourceView* shaderView = nullptr;

		FixedArray<ID3D11RenderTargetView*, 6> renderFaces;

		static CubeMapInstance Create(uint32 resolution);

		//void Bind(RenderState* rs, ShaderStage shaderStage, int32 register_);
		//void Unbind(RenderState* rs);
	};

	struct SamplerState
	{
		TextureFilterMode filter;
		TextureWrapMode wrap;

		ID3D11SamplerState* sampler;

		static void Release(SamplerState* sampler);
		static SamplerState Create(TextureFilterMode filter, TextureWrapMode wrap);
		static SamplerState CreateShadowPFC();
	};

	struct ProgramInstance
	{
		ResourceId id;
		ID3D11VertexShader* vs;
		ID3D11PixelShader* ps;
		ID3D11ComputeShader* cs;
		ID3D11InputLayout* layout;

		static void Release(ProgramInstance* program);
		static ProgramInstance CreateGraphics(const ProgramResource& programResource);
	};

	template<typename T>
	struct ShaderConstBuffer
	{
		T data;
		ID3D11Buffer* buffer;

		inline static void Release(ShaderConstBuffer<T>* constBuffer)
		{
			DeviceContext dc = GetDeviceContext();
			DXRELEASE(constBuffer->buffer);
			GameMemory::ZeroStruct(constBuffer);
		}

		inline static ShaderConstBuffer<T> Create()
		{
			DeviceContext dc = GetDeviceContext();
			D3D11_BUFFER_DESC desc = {};
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			desc.ByteWidth = sizeof(T);
			desc.StructureByteStride = 0;

			ID3D11Buffer* buffer = nullptr;
			DXCHECK(dc.device->CreateBuffer(&desc, NULL, &buffer));

			ShaderConstBuffer<T> result = {};
			result.buffer = buffer;
			result.data = {};

			return result;
		}
	};

#define Float4Align __declspec(align(16))
	struct ShaderConstBufferModel
	{
		Float4Align Mat4f mvp;
		Float4Align Mat4f model;
		Float4Align Mat4f invM;

		inline void Prepare()
		{
			mvp = Transpose(mvp);
			model = Transpose(model);
			invM = Transpose(invM);
		}
	};

	struct ShaderConstBufferView
	{
		Float4Align Mat4f persp;
		Float4Align Mat4f view;
		Float4Align Mat4f screeenProjection;

		inline void Prepare()
		{
			persp = Transpose(persp);
			view = Transpose(view);
			screeenProjection = Transpose(screeenProjection);
		}
	};

	struct ShaderConstBufferLightingInfo
	{
		Float4Align Vec4f viewPos;
		Float4Align struct {
			int32 dirLightCount;
			int32 spotLightCount;
			int32 pointLightCount;
			int32 pad;
		};

		inline void Prepare()
		{

		}
	};

	struct ShaderConstBufferUIData
	{
		Float4Align Vec4f colour;
		Float4Align Vec4f sizePos;
		Float4Align Vec4i uiUses;

		inline void Prepare()
		{

		}
	};

	struct RenderState
	{
		DeviceContext deviceContext;
		SwapChain swapChain;

		union
		{
			ID3D11RasterizerState* allRastersStates[8];
			struct
			{
				ID3D11RasterizerState* rasterBackFaceCullingState;
				ID3D11RasterizerState* rasterFrontFaceCullingState;
				ID3D11RasterizerState* rasterNoFaceCullState;
				ID3D11RasterizerState* rasterNoFaceCullWireframe;
			};
		};

		union
		{
			ID3D11RasterizerState* allDepthStates[8];
			struct
			{
				ID3D11DepthStencilState* depthLessState;
				ID3D11DepthStencilState* depthOffState;
				ID3D11DepthStencilState* depthLessEqualState;
			};
		};

		union
		{
			SamplerState allSampleStates[8];
			struct
			{
				SamplerState pointRepeat;
				SamplerState bilinearRepeat;
				SamplerState trilinearRepeat;
				SamplerState shadowPFC;
			};
		};

		union
		{
			ProgramInstance programs[64];
			struct
			{
				ProgramInstance phongProgram;
				ProgramInstance phongKenneyProgram;
				ProgramInstance postProcessingProgram;
			};
		};


		StaticModel quad;
		StaticModel cube;
		StaticTexture invalidTexture;

		HashMap<StaticModel> staticMeshes;
		HashMap<StaticTexture> textures;

		ShaderConstBuffer<ShaderConstBufferModel> modelConstBuffer;
		ShaderConstBuffer<ShaderConstBufferView> viewConstBuffer;
		ShaderConstBuffer<ShaderConstBufferLightingInfo> lightingConstBuffer;
		ShaderConstBuffer<ShaderConstBufferUIData> uiConstBuffer;

		ID3D11BlendState* blendNormal;
	};


	inline DXGI_FORMAT GetTextureFormatToD3D(const TextureFormat& format)
	{
		switch (format.Get())
		{
		case TextureFormat::Value::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::Value::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
		case TextureFormat::Value::R8_BYTE: return DXGI_FORMAT_R8_UINT;
		case TextureFormat::Value::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::Value::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
		case TextureFormat::Value::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
		case TextureFormat::Value::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
		case TextureFormat::Value::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
		case TextureFormat::Value::R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
		case TextureFormat::Value::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
		case TextureFormat::Value::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
		case TextureFormat::Value::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TextureFormat::Value::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	inline uint32 GetTextureFormatElementSizeBytes(const TextureFormat& format)
	{
		switch (format.Get())
		{
		case TextureFormat::Value::R8G8B8A8_UNORM: return sizeof(uint8);
		case TextureFormat::Value::R16G16_UNORM: return sizeof(uint16);
		case TextureFormat::Value::R8_BYTE: return sizeof(uint8);
		case TextureFormat::Value::R32_FLOAT: return sizeof(real32);
		case TextureFormat::Value::D32_FLOAT: return sizeof(real32);
		case TextureFormat::Value::R32_TYPELESS: return sizeof(real32);
		case TextureFormat::Value::R16_UNORM: return sizeof(uint16);
		case TextureFormat::Value::D16_UNORM: return sizeof(uint16);
		case TextureFormat::Value::R16_TYPELESS: return sizeof(uint16);
		case TextureFormat::Value::R32G32_FLOAT: return sizeof(real32);
		case TextureFormat::Value::R32G32B32_FLOAT: return sizeof(real32);
		case TextureFormat::Value::R32G32B32A32_FLOAT: return sizeof(real32);
		case TextureFormat::Value::R16G16B16A16_FLOAT: return sizeof(uint16);
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return 0;
	}

	inline uint32 GetTextureFormatElementCount(const TextureFormat& format)
	{
		switch (format.Get())
		{
		case TextureFormat::Value::R8G8B8A8_UNORM: return 4;
		case TextureFormat::Value::R16G16_UNORM: return 2;
		case TextureFormat::Value::R8_BYTE: return 1;
		case TextureFormat::Value::R32_FLOAT: return 1;
		case TextureFormat::Value::D32_FLOAT: return 1;
		case TextureFormat::Value::R32_TYPELESS: return 1;
		case TextureFormat::Value::R16_UNORM: return 1;
		case TextureFormat::Value::D16_UNORM: return 1;
		case TextureFormat::Value::R16_TYPELESS: return 1;
		case TextureFormat::Value::R32G32_FLOAT: return 2;
		case TextureFormat::Value::R32G32B32_FLOAT: return 3;
		case TextureFormat::Value::R32G32B32A32_FLOAT: return 4;
		case TextureFormat::Value::R16G16B16A16_FLOAT: return 4;
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return 0;
	}

	inline D3D11_TEXTURE_ADDRESS_MODE GetTextureWrapModeToD3D(const TextureWrapMode& wrap)
	{
		switch (wrap.Get())
		{
		case TextureWrapMode::Value::REPEAT: return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureWrapMode::Value::CLAMP_EDGE:return D3D11_TEXTURE_ADDRESS_CLAMP;
		default: Assert(0, "TextureWrapModeToD3D ??");
		}

		return D3D11_TEXTURE_ADDRESS_WRAP;
	}

	inline D3D11_FILTER GetTextureFilterModeToD3D(const TextureFilterMode& mode)
	{
		switch (mode.Get())
		{
		case TextureFilterMode::Value::POINT:		return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case TextureFilterMode::Value::BILINEAR:	return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TextureFilterMode::Value::TRILINEAR:	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		default: Assert(0, "TextureFilterModeToD3D ??");
		}

		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	inline int32 GetTextureUsageToD3DBindFlags(const BindUsage& usage)
	{
		switch (usage.Get())
		{
		case BindUsage::Value::NONE:  return 0;
		case BindUsage::Value::SHADER_RESOURCE: return D3D11_BIND_SHADER_RESOURCE;
		case BindUsage::Value::RENDER_TARGET: return D3D11_BIND_RENDER_TARGET;
		case BindUsage::Value::DEPTH_SCENCIL_BUFFER: return D3D11_BIND_DEPTH_STENCIL;
		case BindUsage::Value::COMPUTER_SHADER_RESOURCE: return D3D11_BIND_UNORDERED_ACCESS;
		default: Assert(0, "TextureUsageToD3DBindFlags ??");
		}

		return 0;
	}

	inline int32 GetCPUFlagsToD3DFlags(const ResourceCPUFlags& flags)
	{
		switch (flags.Get())
		{
		case ResourceCPUFlags::Value::NONE: return 0;
		case ResourceCPUFlags::Value::READ: return D3D11_CPU_ACCESS_READ;
		case ResourceCPUFlags::Value::WRITE: return D3D11_CPU_ACCESS_WRITE;
		case ResourceCPUFlags::Value::READ_WRITE: return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		}
		return 0;
	}

}