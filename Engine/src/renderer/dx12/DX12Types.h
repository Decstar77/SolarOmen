#pragma once
#include "SolarDefines.h"
#include "renderer/RendererTypes.h"
#include "resources/SolarResourceTypes.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>

#define DirectXDebugMessageCount 10
#if SOL_DEBUG_RENDERING
#define DXCHECK(call)                                                                                                   \
    {                                                                                                                   \
        RenderDebug::next = RenderDebug::info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);                    \
        HRESULT dxresult = (call);                                                                                      \
        if (FAILED(dxresult))                                                                                           \
        {                                                                                                               \
            char *output = nullptr;                                                                                     \
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, \
                           NULL, dxresult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&output, 0, NULL);         \
            if (output)                                                                                                 \
            {                                                                                                           \
                LogDirectXDebugGetMessages();															                \
                debugBreak();                                                                                           \
            }                                                                                                           \
        }                                                                                                               \
    }

#define DXINFO(call)                                         \
    {                                                        \
        RenderDebug::next = RenderDebug::info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL); \
		(call);                                              \
        LogDirectXDebugGetMessages();		                 \
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
#if SOL_DEBUG_RENDERING
	struct RenderDebug
	{
		inline static uint64 next = 0;
		inline static IDXGIInfoQueue* info_queue = nullptr;
		inline static ID3D12Debug3* debugController = nullptr;
		//ShaderInstance shader;
		//ID3D11Buffer* vertex_buffer;
	};

	void LogDirectXDebugGetMessages();
#endif

	struct RenderState;

	struct Fence
	{
		ID3D12Fence* fence;
		uint64 value;
	};

	struct Command
	{
		ID3D12CommandQueue* queue;
		ID3D12CommandAllocator* allocator[3];
		ID3D12GraphicsCommandList* list;
	};

	struct StaticProgram
	{
		VertexLayoutType layout;
		D3D12_SHADER_BYTECODE vertexShaderByteCode;
		D3D12_SHADER_BYTECODE pixelShaderByteCode;

#if SOL_DEBUG_RENDERING
		static StaticProgram DebugCreateCompile(const String& programPath, VertexLayoutType layout);
#endif
	};

	struct StaticMesh
	{
		ID3D12Resource* vertexBuffer;
		ID3D12Resource* indexBuffer;

		uint32 vertexCount;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		uint32 indexCount;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		static StaticMesh Create(real32* vertices, uint32 vertexCount, VertexLayoutType layout);
		static StaticMesh Create(real32* vertices, uint32 vertexCount, uint32* indices, uint32 indexCount, VertexLayoutType layout);
	};

	struct StaticTexture
	{
		ID3D12Resource* texture;

		static StaticTexture Create(uint32 width, uint32 height, TextureFormat format);
	};

	struct RenderState
	{
		ID3D12Device* device;

		Command command;

		HANDLE fenceEvent;
		Fence fences[3];

		ID3D12DescriptorHeap* rtvDescriptorHeap;
		uint32 rtvDescriptorSize;
		ID3D12DescriptorHeap* dsvDescriptorHeap;
		uint32 dsvDescriptorSize;
		uint32 cbvSrvUavDescriptorSize;

		uint32 swapChainBufferCount;
		IDXGISwapChain3* swapChain;
		ID3D12Resource* swapChainBuffers[3];
		uint32 currentSwapChainBufferIndex;

		ID3D12PipelineState* pso;
		ID3D12RootSignature* rootSignature;

		ID3D12DescriptorHeap* mainDescriptorHeap[3];
		ID3D12Resource* constantBufferUploadHeap[3];
		uint8* cbColorMultiplierGPUAddress[3];
		Vec4f cbColour;


		StaticMesh mesh;

		static ID3D12Device* GetDevice();
		static ID3D12CommandAllocator* GetCurrentCommandAllocator();
		static ID3D12GraphicsCommandList* GetCommandList();
		static void ExecuteCommandList();
		static void FlushCommandQueue();
		static void ResourceTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES end);
	};
}