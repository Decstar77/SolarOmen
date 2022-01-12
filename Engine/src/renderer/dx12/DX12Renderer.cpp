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
	static RenderState rs = {};
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

	ID3D12Device* RenderState::GetDevice()
	{
		return rs.device;
	}

	ID3D12CommandAllocator* RenderState::GetCurrentCommandAllocator()
	{
		return rs.command.allocator[rs.currentSwapChainBufferIndex];
	}

	ID3D12GraphicsCommandList* RenderState::GetCommandList()
	{
		return rs.command.list;
	}

	void RenderState::ExecuteCommandList()
	{
		ID3D12CommandList* commandLists[] = { rs.command.list };
		DXINFO(rs.command.queue->ExecuteCommandLists(1, commandLists));
		DXCHECK(rs.command.queue->Signal(rs.fences[rs.currentSwapChainBufferIndex].fence,
			rs.fences[rs.currentSwapChainBufferIndex].value));
	}

	void RenderState::FlushCommandQueue()
	{
		Fence* fence = &rs.fences[rs.currentSwapChainBufferIndex];

		if (fence->fence->GetCompletedValue() < fence->value)
		{
			DXCHECK(fence->fence->SetEventOnCompletion(fence->value, rs.fenceEvent));
			WaitForSingleObject(rs.fenceEvent, INFINITE);
		}

		fence->value++;
	}

	void RenderState::ResourceTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES end)
	{
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, end);
		DXINFO(rs.command.list->ResourceBarrier(1, &transition));
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
		DXCHECK(rs.device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&sig)));
		signature->Release();

		return sig;
	}



	inline static void OffsetRTVDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* handle, int32 offset) { handle->ptr += offset * rs.rtvDescriptorSize; }

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
			rs.swapChainBufferCount = 3;

			DXCHECK(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&rs.device)));

			D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
			commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

			DXCHECK(rs.device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&rs.command.queue)));
			for (uint32 i = 0; i < rs.swapChainBufferCount; i++)
			{
				DXCHECK(rs.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&rs.command.allocator[i])));
			}

			DXCHECK(rs.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				rs.command.allocator[0], nullptr, IID_PPV_ARGS(&rs.command.list)));
			DXCHECK(rs.command.list->Close());

			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = rs.swapChainBufferCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.NodeMask = 0;
			DXCHECK(rs.device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rs.rtvDescriptorHeap)));

			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsvHeapDesc.NodeMask = 0;
			DXCHECK(rs.device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&rs.dsvDescriptorHeap)));

			rs.rtvDescriptorSize = rs.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			rs.dsvDescriptorSize = rs.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			rs.cbvSrvUavDescriptorSize = rs.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.BufferDesc.Width = Platform::GetWindowWidth();
			swapChainDesc.BufferDesc.Height = Platform::GetWindowHeight();
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = rs.swapChainBufferCount;
			swapChainDesc.OutputWindow = (HWND)Platform::GetNativeState();
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			IDXGISwapChain* tempSwapChain = nullptr;
			DXCHECK(dxgiFactory->CreateSwapChain(rs.command.queue, &swapChainDesc, &tempSwapChain));
			rs.swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rs.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			for (uint32 i = 0; i < rs.swapChainBufferCount; i++)
			{
				DXCHECK(rs.swapChain->GetBuffer(i, IID_PPV_ARGS(&rs.swapChainBuffers[i])));
				DXINFO(rs.device->CreateRenderTargetView(rs.swapChainBuffers[i], nullptr, rtvHandle));
				OffsetRTVDescriptor(&rtvHandle, 1);
			}

			for (uint32 i = 0; i < rs.swapChainBufferCount; i++)
			{
				DXCHECK(rs.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&rs.fences[i].fence)));
				rs.fences[i].value = 0;
			}


			rs.fenceEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);

			SOLINFO("DirectX12 device/swapchain/queue have been successfully created");


			StaticProgram program = StaticProgram::DebugCreateCompile(
				"F:/codes/SolarOmen/SolarOmen-2/Engine/src/renderer/shaders/FirstShader.hlsl",
				VertexLayoutType::Value::PC);

			rs.rootSignature = CreateRootSignature(nullptr, 0, nullptr,
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
			psoDesc.pRootSignature = rs.rootSignature;
			psoDesc.VS = program.vertexShaderByteCode;
			psoDesc.PS = program.pixelShaderByteCode;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleMask = 0xffffffff;
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.NumRenderTargets = 1;
			//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			//psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			DXCHECK(rs.device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&rs.pso)));

			real32 vertices[] = {
				0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f ,
				0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f ,
				-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f
			};

			rs.mesh = StaticMesh::Create(vertices, 3, VertexLayoutType::Value::PC);


			return true;
		}

		return false;
	}

	void Renderer::Render(RenderPacket* renderPacket)
	{
		RenderState::FlushCommandQueue();
		rs.currentSwapChainBufferIndex = rs.swapChain->GetCurrentBackBufferIndex();

		DXCHECK(rs.command.allocator[rs.currentSwapChainBufferIndex]->Reset());
		DXCHECK(rs.command.list->Reset(rs.command.allocator[rs.currentSwapChainBufferIndex], rs.pso));

		auto renderTarget = rs.swapChainBuffers[rs.currentSwapChainBufferIndex];
		RenderState::ResourceTransition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rs.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			rs.currentSwapChainBufferIndex, rs.rtvDescriptorSize);

		/////////////////////////////////////

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (real32)Platform::GetWindowWidth();
		viewport.Height = (real32)Platform::GetWindowHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Platform::GetWindowWidth();
		scissorRect.bottom = Platform::GetWindowHeight();

		Vec4f cc = Vec4f(0.0f, 0.2f, 0.4f, 1.0f);
		DXINFO(rs.command.list->ClearRenderTargetView(rtvHandle, cc.ptr, 0, nullptr));
		DXINFO(rs.command.list->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL));
		DXINFO(rs.command.list->SetGraphicsRootSignature(rs.rootSignature));
		DXINFO(rs.command.list->RSSetViewports(1, &viewport));
		DXINFO(rs.command.list->RSSetScissorRects(1, &scissorRect));
		DXINFO(rs.command.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		DXINFO(rs.command.list->IASetVertexBuffers(0, 1, &rs.mesh.vertexBufferView));
		DXINFO(rs.command.list->DrawInstanced(3, 1, 0, 0));

		RenderState::ResourceTransition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		/////////////////////////////////////
		DXCHECK(rs.command.list->Close());
		ID3D12CommandList* commandLists[] = { rs.command.list };
		DXINFO(rs.command.queue->ExecuteCommandLists(1, commandLists));
		DXCHECK(rs.command.queue->Signal(rs.fences[rs.currentSwapChainBufferIndex].fence,
			rs.fences[rs.currentSwapChainBufferIndex].value));
		DXCHECK(rs.swapChain->Present(0, 0));
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