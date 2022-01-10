#pragma once
#include "../SolarRenderer.h"
#include "core/SolarEvent.h"
#include "core/SolarLogging.h"
#if SOLAR_PLATFORM_WINDOWS && USE_DIRECX12

#include "platform/SolarPlatform.h"
#include "platform/win32/Win32State.h"

#include "DX12Types.h"
#include "d3dx12.h"
//#include "DX11RenderCommands.h"

namespace sol
{
	static RenderState renderState = {};
#if SOL_DEBUG_RENDERING
	static bool8 InitializeDirectXDebugLogging()
	{
		HRESULT dxresult = {};
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		HMODULE mod_debug = LoadLibraryExA("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (mod_debug)
		{
			DXGIGetDebugInterface debug_fnc = reinterpret_cast<DXGIGetDebugInterface>(
				reinterpret_cast<void*>(GetProcAddress(mod_debug, "DXGIGetDebugInterface")));
			if (debug_fnc)
			{
				dxresult = debug_fnc(__uuidof(IDXGIInfoQueue), (void**)&RenderDebug::info_queue);
				return true;
			}
			else
			{
				SOLERROR("Could not find DXGIGetDebugInterface in dxgidebug.dll!");
			}
		}
		else
		{
			SOLERROR("Could not find dxgidebug.dll!");
		}

		return false;
	}

	void LogDirectXDebugGetMessages()
	{
		uint64 end = RenderDebug::info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (uint64 i = RenderDebug::next; i < end; i++)
		{
			SIZE_T messageLength = 0;
			RenderDebug::info_queue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength);

			byte* bytes = GameMemory::PushTransientCount<byte>((uint32)messageLength);
			DXGI_INFO_QUEUE_MESSAGE* message = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes);

			RenderDebug::info_queue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength);

			SOLERROR(message->pDescription);
		}
	}
#endif

	ID3D12Device* sol::GetDevice()
	{
		return renderState.device;
	}

	IDXGIAdapter1* FindSuitableGPUAdapter(IDXGIFactory4* dxgiFactory)
	{
		IDXGIAdapter1* adapter = nullptr;
		int32 adapterIndex = 0;

		while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc = {};
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				char output[256];
				sprintf_s(output, 256, "%ws", desc.Description);

				SOLINFO(output);

				break;
			}
			adapterIndex++;
		}

		if (!adapter)
		{
			SOLFATAL("No GPU suitable for game !!");
		}

		return adapter;
	}

	static ID3D12RootSignature* CreateRootSignature(D3D12_ROOT_PARAMETER* params, uint32 paramCount,
		D3D12_STATIC_SAMPLER_DESC* samplers, uint32 samplerCount, D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = paramCount;
		desc.pParameters = params;
		desc.NumStaticSamplers = samplerCount;
		desc.pStaticSamplers = samplers;
		desc.Flags = flags;

		ID3DBlob* signature = nullptr;
		ID3DBlob* errorBlob = nullptr;

		ID3D12RootSignature* sig = nullptr;

		DXCHECK(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBlob));
		DXCHECK(renderState.device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&sig)));
		signature->Release();

		return sig;
	}


	static void FlushCommandQueue()
	{
		Fence* fence = &renderState.fences[renderState.currentSwapChainBufferIndex];

		uint64 c = fence->fence->GetCompletedValue();

		if (c < fence->value)
		{
			DXCHECK(fence->fence->SetEventOnCompletion(fence->value, renderState.fenceEvent));
			WaitForSingleObject(renderState.fenceEvent, INFINITE);
		}

		c = fence->fence->GetCompletedValue();
		fence->value++;
	}

	inline static void OffsetRTVDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* handle, int32 offset) { handle->ptr += offset * renderState.rtvDescriptorSize; }

	bool8 Renderer::Initialize()
	{
#if SOL_DEBUG_RENDERING
		if (!InitializeDirectXDebugLogging())
			return false;

		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&RenderDebug::debugController))))
		{
			RenderDebug::debugController->EnableDebugLayer();
			RenderDebug::debugController->SetEnableGPUBasedValidation(true);
			RenderDebug::debugController->SetEnableSynchronizedCommandQueueValidation(true);
			SOLINFO("Created Direcx12 debug");
		}
		else
		{
			SOLERROR("Could create debug interface");
			return false;
		}
#endif
		IDXGIFactory4* dxgiFactory = nullptr;
		DXCHECK(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

		IDXGIAdapter1* adapter = FindSuitableGPUAdapter(dxgiFactory);
		if (adapter)
		{
			renderState.swapChainBufferCount = 3;

			DXCHECK(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&renderState.device)));

			D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
			commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

			DXCHECK(renderState.device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&renderState.commandQueue)));
			for (uint32 i = 0; i < renderState.swapChainBufferCount; i++)
			{
				DXCHECK(renderState.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&renderState.commandAllocator[i])));
			}

			DXCHECK(renderState.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				renderState.commandAllocator[0], nullptr, IID_PPV_ARGS(&renderState.commandList)));
			DXCHECK(renderState.commandList->Close());

			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = renderState.swapChainBufferCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.NodeMask = 0;
			DXCHECK(renderState.device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&renderState.rtvDescriptorHeap)));

			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsvHeapDesc.NodeMask = 0;
			DXCHECK(renderState.device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&renderState.dsvDescriptorHeap)));

			renderState.rtvDescriptorSize = renderState.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			renderState.dsvDescriptorSize = renderState.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			renderState.cbvSrvUavDescriptorSize = renderState.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.BufferDesc.Width = Platform::GetWindowWidth();
			swapChainDesc.BufferDesc.Height = Platform::GetWindowHeight();
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = renderState.swapChainBufferCount;
			swapChainDesc.OutputWindow = (HWND)Platform::GetNativeState();
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			IDXGISwapChain* tempSwapChain = nullptr;
			DXCHECK(dxgiFactory->CreateSwapChain(renderState.commandQueue, &swapChainDesc, &tempSwapChain));
			renderState.swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderState.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			for (uint32 i = 0; i < renderState.swapChainBufferCount; i++)
			{
				DXCHECK(renderState.swapChain->GetBuffer(i, IID_PPV_ARGS(&renderState.swapChainBuffers[i])));
				DXINFO(renderState.device->CreateRenderTargetView(renderState.swapChainBuffers[i], nullptr, rtvHandle));
				OffsetRTVDescriptor(&rtvHandle, 1);
			}

			for (uint32 i = 0; i < renderState.swapChainBufferCount; i++)
			{
				DXCHECK(renderState.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&renderState.fences[i].fence)));
				renderState.fences[i].value = 0;
			}


			renderState.fenceEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);

			SOLINFO("DirectX12 device/swapchain/queue have been successfully created");


			StaticProgram program = StaticProgram::DebugCreateCompile(
				"F:/codes/SolarOmen/SolarOmen-2/Engine/src/renderer/shaders/FirstShader.hlsl",
				VertexLayoutType::Value::PC);

			ID3D12RootSignature* rootSignature = CreateRootSignature(nullptr, 0, nullptr,
				0, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			D3D12_INPUT_ELEMENT_DESC inputLayout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOUR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
			inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
			inputLayoutDesc.pInputElementDescs = inputLayout;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = inputLayoutDesc;
			psoDesc.pRootSignature = rootSignature;
			psoDesc.VS = program.vertexShaderByteCode;
			psoDesc.PS = program.pixelShaderByteCode;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleMask = 0xffffffff;
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.NumRenderTargets = 1;
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			DXCHECK(renderState.device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&renderState.pso)));

			return true;
		}

		return false;
	}

	static void ResourceTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES end)
	{
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, end);
		DXINFO(renderState.commandList->ResourceBarrier(1, &transition));
	}

	void Renderer::Render(RenderPacket* renderPacket)
	{
		FlushCommandQueue();
		renderState.currentSwapChainBufferIndex = renderState.swapChain->GetCurrentBackBufferIndex();

		DXCHECK(renderState.commandAllocator[renderState.currentSwapChainBufferIndex]->Reset());
		DXCHECK(renderState.commandList->Reset(renderState.commandAllocator[renderState.currentSwapChainBufferIndex], NULL));

		auto renderTarget = renderState.swapChainBuffers[renderState.currentSwapChainBufferIndex];
		ResourceTransition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(renderState.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			renderState.currentSwapChainBufferIndex, renderState.rtvDescriptorSize);

		/////////////////////////////////////

		Vec4f cc = Vec4f(0.0f, 0.2f, 0.4f, 1.0f);
		DXINFO(renderState.commandList->ClearRenderTargetView(rtvHandle, cc.ptr, 0, nullptr));
		DXINFO(renderState.commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL));

		ResourceTransition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		/////////////////////////////////////
		DXCHECK(renderState.commandList->Close());
		ID3D12CommandList* commandLists[] = { renderState.commandList };
		DXINFO(renderState.commandQueue->ExecuteCommandLists(1, commandLists));
		DXCHECK(renderState.commandQueue->Signal(renderState.fences[renderState.currentSwapChainBufferIndex].fence,
			renderState.fences[renderState.currentSwapChainBufferIndex].value));
		DXCHECK(renderState.swapChain->Present(0, 0));
	}

	void Renderer::Shutdown()
	{

	}

	void* Renderer::GetNativeDeviceContext()
	{
		return nullptr;
	}
}


#endif