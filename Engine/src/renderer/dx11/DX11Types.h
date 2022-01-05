#pragma once
#include "SolarDefines.h"
#include "renderer/RendererTypes.h"
#include "resources/SolarResourceTypes.h"

#include <d3d11.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

#define DirectXDebugMessageCount 10

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
		IDXGISwapChain* swapChain;
		ID3D11Texture2D* depthTexture;
		ID3D11DepthStencilView* depthView;
		ID3D11ShaderResourceView* depthShaderView;
		ID3D11RenderTargetView* renderView;
	};

	struct RenderDebug
	{
		uint64 next;
		struct IDXGIInfoQueue* info_queue;
		//ShaderInstance shader;
		//ID3D11Buffer* vertex_buffer;
	};

	void LogDirectXDebugGetMessages(RenderDebug* debug);

	struct DeviceContext
	{
		ID3D11Device* device;
		ID3D11DeviceContext* context;
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

	struct StaticMesh
	{
		uint32 strideBytes; // @NOTE: Used for rendering
		uint32 indexCount;  // @NOTE: Used for rendering
		uint32 vertexCount;  // @NOTE: Used for rendering
		VertexLayoutType vertexLayout;

		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;

		//void UpdateVertexBuffer(real32* vertices, uint32 sizeBytes);

		//static StaticMesh Create(const ModelAsset& modelAsset);
		static void Release(StaticMesh* mesh);
		static StaticMesh Create(real32* vertices, uint32 vertexCount, VertexLayoutType layout);
		static StaticMesh Create(real32* vertices, uint32 vertexCount, uint32* indices, uint32 indexCount, VertexLayoutType layout);
		static StaticMesh CreateScreenSpaceQuad();
		static StaticMesh CreateUnitCube();
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
			ProgramInstance programs[64];
			struct
			{
				ProgramInstance phongProgram;
				ProgramInstance postProcessingProgram;
			};
		};

		StaticMesh quad;
		StaticMesh cube;
		ManagedArray<StaticMesh> staticMeshes;


		ShaderConstBuffer<ShaderConstBufferModel> modelConstBuffer;
		ShaderConstBuffer<ShaderConstBufferView> viewConstBuffer;
		ShaderConstBuffer<ShaderConstBufferLightingInfo> lightingConstBuffer;
		ShaderConstBuffer<ShaderConstBufferUIData> uiConstBuffer;

		ID3D11BlendState* blendNormal;
	};

}