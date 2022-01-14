#pragma once
#include "../SolarRenderer.h"
#include "core/SolarEvent.h"
#include "core/SolarLogging.h"
#include "core/SolarDebug.h"

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

	void RenderState::FlushCommandQueue(bool8 incrementFenceValue)
	{
		Fence* fence = &rs.fences[rs.currentSwapChainBufferIndex];

		if (fence->fence->GetCompletedValue() < fence->value)
		{
			//SOLTRACE("WAIT");
			DXCHECK(fence->fence->SetEventOnCompletion(fence->value, rs.fenceEvent));
			WaitForSingleObject(rs.fenceEvent, INFINITE);
		}

		if (incrementFenceValue)
		{
			fence->value++;
		}
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

		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBlob);

		if (FAILED(hr))
		{
			SOLERROR((char*)errorBlob->GetBufferPointer());
			return nullptr;
		}

		DXCHECK(rs.device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&sig)));

		DXRELEASE(errorBlob);
		DXRELEASE(signature);

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

			D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 10;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			dsvHeapDesc.NodeMask = 0;
			DXCHECK(rs.device->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&rs.cbvSrvUavDescriptorHeap)));

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


			D3D12_ROOT_PARAMETER  rootParameters[3] = {};
			rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParameters[0].Descriptor.RegisterSpace = 0;
			rootParameters[0].Descriptor.ShaderRegister = 0;
			rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParameters[1].Constants.RegisterSpace = 0;
			rootParameters[1].Constants.ShaderRegister = 1;
			rootParameters[1].Constants.Num32BitValues = 4;
			rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1] = {};
			descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorTableRanges[0].NumDescriptors = 1;
			descriptorTableRanges[0].BaseShaderRegister = 0;
			descriptorTableRanges[0].RegisterSpace = 0;
			descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
			rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorTableRanges[0];
			rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_STATIC_SAMPLER_DESC sampler = {};
			sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.MipLODBias = 0;
			sampler.MaxAnisotropy = 0;
			sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			sampler.MinLOD = 0.0f;
			sampler.MaxLOD = D3D12_FLOAT32_MAX;
			sampler.ShaderRegister = 0;
			sampler.RegisterSpace = 0;
			sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			rs.rootSignature = CreateRootSignature(
				rootParameters, ArrayCount(rootParameters),
				&sampler, 1,
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

			for (uint32 i = 0; i < rs.swapChainBufferCount; i++)
			{
				auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64);
				DXCHECK(rs.device->CreateCommittedResource(
					&heapType,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&rs.constantBufferUploadHeap[i])));

				CD3DX12_RANGE readRange(0, 0);
				DXCHECK(rs.constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&rs.cbColorMultiplierGPUAddress[i])));

				rs.mvp1 = Mat4f(1);
				rs.mvp2 = Mat4f(1);

				memcpy(rs.cbColorMultiplierGPUAddress[i], &rs.mvp1.ptr, sizeof(rs.mvp1));
				memcpy(rs.cbColorMultiplierGPUAddress[i] + ConstantBufferPerObjectAlignedSize, &rs.mvp2.ptr, sizeof(rs.mvp2));
			}

			D3D12_INPUT_ELEMENT_DESC inputLayout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout.NumElements = ArrayCount(inputLayout);
			psoDesc.InputLayout.pInputElementDescs = inputLayout;
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
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			DXCHECK(rs.device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&rs.pso)));

			StaticTexture depthTexture = StaticTexture::Create(Platform::GetWindowWidth(), Platform::GetWindowHeight(), TextureFormat::Value::D32_FLOAT);
			TextureResource textureRes = *Resources::GetTextureResource("PolygonScifi_01_C");
			rs.texture = StaticTexture::Create(textureRes);

			//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			//srvDesc.Format = GetTextureFormatToD3D(textureRes.format);
			//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			//srvDesc.Texture2D.MipLevels = 1;

			//(rs.device->CreateShaderResourceView(rs.texture.texture, &srvDesc, rs.cbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart()));

			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
			DXINFO(rs.device->CreateDepthStencilView(depthTexture.texture, &depthStencilDesc, rs.dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()));


			//real32 vertices[] = {
			//	0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f ,
			//	0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f ,
			//	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f
			//};

			//rs.mesh = StaticMesh::Create(vertices, 3, VertexLayoutType::Value::PC);

			real32 vertexData[] = {
				-1, 1, 0,	0, 0, -1,	0, 0,
				1, -1, 0,	0, 0, -1,	1, 1,
				-1, -1, 0,	0, 0, -1,	0, 1,
				1, 1, 0,	0, 0, -1,	1, 0
			};

			uint32 indexData[] = {
				0, 1, 2, 0, 3, 1
			};

			rs.mesh = StaticMesh::Create(vertexData, 4, indexData, 6, VertexLayoutType::Value::PNT);



			for (uint32 i = 0; i < rs.swapChainBufferCount; i++) { rs.fences[i].value = 0; }

			return true;
		}

		return false;
	}

	void Renderer::Render(RenderPacket* renderPacket)
	{
		rs.currentSwapChainBufferIndex = rs.swapChain->GetCurrentBackBufferIndex();

		Transform v = Transform(Vec3f(0, 0, -3));
		Transform m1 = Transform(Vec3f(-1.5f, 0, 0));
		Transform m2 = Transform(Vec3f(1.5f, 0, 0));
		rs.mvp1 = Transpose(m1.CalculateTransformMatrix() * Inverse(v.CalculateTransformMatrix()) * renderPacket->projectionMatrix);
		rs.mvp2 = Transpose(m2.CalculateTransformMatrix() * Inverse(v.CalculateTransformMatrix()) * renderPacket->projectionMatrix);

		memcpy(rs.cbColorMultiplierGPUAddress[rs.currentSwapChainBufferIndex], &rs.mvp1.ptr, sizeof(rs.mvp1));
		memcpy(rs.cbColorMultiplierGPUAddress[rs.currentSwapChainBufferIndex] + ConstantBufferPerObjectAlignedSize, &rs.mvp2.ptr, sizeof(rs.mvp2));

		RenderState::FlushCommandQueue(true);
		DXCHECK(rs.command.allocator[rs.currentSwapChainBufferIndex]->Reset());
		DXCHECK(rs.command.list->Reset(rs.command.allocator[rs.currentSwapChainBufferIndex], rs.pso));

		auto renderTarget = rs.swapChainBuffers[rs.currentSwapChainBufferIndex];
		RenderState::ResourceTransition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rs.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			rs.currentSwapChainBufferIndex, rs.rtvDescriptorSize);

		auto dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rs.dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			0, rs.dsvDescriptorSize);
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
		DXINFO(rs.command.list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr));
		DXINFO(rs.command.list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle));
		DXINFO(rs.command.list->RSSetViewports(1, &viewport));
		DXINFO(rs.command.list->RSSetScissorRects(1, &scissorRect));
		DXINFO(rs.command.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		DXINFO(rs.command.list->IASetVertexBuffers(0, 1, &rs.mesh.vertexBufferView));
		DXINFO(rs.command.list->IASetIndexBuffer(&rs.mesh.indexBufferView));
		DXINFO(rs.command.list->SetGraphicsRootSignature(rs.rootSignature));


		DXINFO(rs.command.list->SetGraphicsRootConstantBufferView(0, rs.constantBufferUploadHeap[rs.currentSwapChainBufferIndex]->GetGPUVirtualAddress()));
		Vec4f colour = Vec4f(1, 1, 1, 1);
		DXINFO(rs.command.list->SetGraphicsRoot32BitConstants(1, 4, colour.ptr, 0));

		//rs.command.list->SetGraphicsRootDescriptorTable(2, )


		DXINFO(rs.command.list->DrawIndexedInstanced(rs.mesh.indexCount, 1, 0, 0, 0));

		DXINFO(rs.command.list->SetGraphicsRootConstantBufferView(0,
			rs.constantBufferUploadHeap[rs.currentSwapChainBufferIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize));
		colour = Vec4f(1, 0, 1, 1);
		DXINFO(rs.command.list->SetGraphicsRoot32BitConstants(1, 4, colour.ptr, 0));
		DXINFO(rs.command.list->DrawIndexedInstanced(rs.mesh.indexCount, 1, 0, 0, 0));

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