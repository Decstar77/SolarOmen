#include "renderer/RendererFrontEnd.h"
#include "core/SolarEvent.h"
#include "core/SolarLogging.h"
#if SOLAR_PLATFORM_WINDOWS

#include "platform/SolarPlatform.h"
#include "platform/win32/Win32State.h"

#include "DX11Types.h"
#include "DX11RenderCommands.h"

namespace sol
{
	static RenderState renderState = {};

	DeviceContext GetDeviceContext()
	{
		return renderState.deviceContext;
	}

	static void InitializeDirectXDebugLogging()
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
				SOLTRACE("Intialized directx 11 debug logging")
					dxresult = debug_fnc(__uuidof(IDXGIInfoQueue), (void**)&renderState.deviceContext.debug.info_queue);
			}
			else
			{
				SOLERROR("Could not intialize directx 11 debug logging");
			}
		}
		else
		{
			SOLERROR("Could not intialize directx 11 debug logging");
		}
	}

	void LogDirectXDebugGetMessages(RenderDebug* debug)
	{
		uint64 end = debug->info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (uint64 i = debug->next; i < end; i++)
		{
			SIZE_T messageLength = 0;
			debug->info_queue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength);

			byte* bytes = GameMemory::PushTransientCount<byte>((uint32)messageLength);
			DXGI_INFO_QUEUE_MESSAGE* message = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes);

			debug->info_queue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength);

			SOLERROR(message->pDescription);
		}
	}

	static void CreateSwapChainBuffers()
	{
		DeviceContext dc = renderState.deviceContext;
		Win32State* win32State = (Win32State*)Platform::GetInternalState();

		// @NOTE: Get back buffer
		ID3D11Resource* backBuffer = nullptr;
		DXCHECK(renderState.swapChain.swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backBuffer));
		DXCHECK(renderState.deviceContext.device->CreateRenderTargetView(backBuffer, nullptr, &renderState.swapChain.renderView));
		backBuffer->Release();

		RECT windowRect = {};
		GetClientRect(win32State->window, &windowRect);

		D3D11_TEXTURE2D_DESC depthDesc = {};
		depthDesc.Width = (uint32)(windowRect.right - windowRect.left);
		depthDesc.Height = (uint32)(windowRect.bottom - windowRect.top);
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		DXINFO(renderState.deviceContext.device->CreateTexture2D(&depthDesc, nullptr, &renderState.swapChain.depthTexture));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
		depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthViewDesc.Texture2D.MipSlice = 0;
		DXCHECK(renderState.deviceContext.device->CreateDepthStencilView(renderState.swapChain.depthTexture, &depthViewDesc, &renderState.swapChain.depthView));

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {};
		shaderViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderViewDesc.Texture2D.MostDetailedMip = 0;
		shaderViewDesc.Texture2D.MipLevels = 1;
		DXCHECK(renderState.deviceContext.device->CreateShaderResourceView(renderState.swapChain.depthTexture, &shaderViewDesc, &renderState.swapChain.depthShaderView));
	}

	static void CreateAllRasterState()
	{
		DeviceContext dc = renderState.deviceContext;

		{
			D3D11_RASTERIZER_DESC rsDesc = {};
			rsDesc.FillMode = D3D11_FILL_SOLID;
			rsDesc.CullMode = D3D11_CULL_BACK;
			rsDesc.FrontCounterClockwise = FALSE;
			rsDesc.DepthBias = 0;
			rsDesc.DepthBiasClamp = 1.0f;
			rsDesc.SlopeScaledDepthBias = 0.0f;
			rsDesc.DepthClipEnable = TRUE;
			rsDesc.MultisampleEnable = FALSE;
			rsDesc.AntialiasedLineEnable = FALSE;

			DXCHECK(dc.device->CreateRasterizerState(&rsDesc, &renderState.rasterBackFaceCullingState));
		}

		{
			D3D11_RASTERIZER_DESC rsDesc = {};
			rsDesc.FillMode = D3D11_FILL_SOLID;
			rsDesc.CullMode = D3D11_CULL_FRONT;
			rsDesc.FrontCounterClockwise = FALSE;
			rsDesc.DepthBias = 0;
			rsDesc.DepthBiasClamp = 1.0f;
			rsDesc.SlopeScaledDepthBias = 0.0f;
			rsDesc.DepthClipEnable = TRUE;
			rsDesc.MultisampleEnable = FALSE;
			rsDesc.AntialiasedLineEnable = FALSE;
			DXCHECK(dc.device->CreateRasterizerState(&rsDesc, &renderState.rasterFrontFaceCullingState));
		}

		{
			D3D11_RASTERIZER_DESC rsDesc = {};
			rsDesc.FillMode = D3D11_FILL_SOLID;
			rsDesc.CullMode = D3D11_CULL_NONE;
			rsDesc.FrontCounterClockwise = FALSE;
			rsDesc.DepthBias = 0;
			rsDesc.DepthBiasClamp = 1.0f;
			rsDesc.SlopeScaledDepthBias = 0.0f;
			rsDesc.DepthClipEnable = TRUE;
			rsDesc.MultisampleEnable = FALSE;
			rsDesc.AntialiasedLineEnable = FALSE;
			DXCHECK(dc.device->CreateRasterizerState(&rsDesc, &renderState.rasterNoFaceCullState));
		}
	}

	static void CreateAllDepthStencilState()
	{
		DeviceContext dc = renderState.deviceContext;

		{
			D3D11_DEPTH_STENCIL_DESC ds = {};
			ds.DepthEnable = TRUE;
			ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			ds.DepthFunc = D3D11_COMPARISON_LESS;
			ds.StencilEnable = FALSE;

			DXCHECK(dc.device->CreateDepthStencilState(&ds, &renderState.depthLessState));
		}

		{
			D3D11_DEPTH_STENCIL_DESC ds = {};
			ds.DepthEnable = FALSE;
			ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			ds.DepthFunc = D3D11_COMPARISON_ALWAYS;
			ds.StencilEnable = FALSE;
			DXCHECK(dc.device->CreateDepthStencilState(&ds, &renderState.depthOffState));
		}

		{
			D3D11_DEPTH_STENCIL_DESC ds = {};
			ds.DepthEnable = TRUE;
			ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			ds.StencilEnable = FALSE;
			DXCHECK(dc.device->CreateDepthStencilState(&ds, &renderState.depthLessEqualState));
		}
	}

	static void CreateAllBlendState()
	{
		DeviceContext dc = renderState.deviceContext;

		{
			D3D11_BLEND_DESC blendDesc = {};
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			DXCHECK(dc.device->CreateBlendState(&blendDesc, &renderState.blendNormal));
			dc.context->OMSetBlendState(renderState.blendNormal, nullptr, 0xffffffff);
		}
	}

	bool8 OnWindowResizeCallback(uint16 eventCode, void* sender, void* listener, EventContext context)
	{
		EventWindowResize* resizeEvent = (EventWindowResize*)&context;

		RenderCommand::SetRenderTargets(0, 0);

		DXRELEASE(renderState.swapChain.renderView);
		DXRELEASE(renderState.swapChain.depthView);
		DXRELEASE(renderState.swapChain.depthShaderView);
		DXRELEASE(renderState.swapChain.depthTexture);

		DeviceContext dc = renderState.deviceContext;
		DXCHECK(renderState.swapChain.swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

		CreateSwapChainBuffers();

		return 0;
	}

	bool8 Renderer::Initialize()
	{
#if SOL_DEBUG_RENDERING
		InitializeDirectXDebugLogging();
#endif
		Win32State* win32State = (Win32State*)Platform::GetInternalState();

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc.Width = 0;
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1000;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60000;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // @NOTE This implies just the back buffer, ie we have two buffers
		swapChainDesc.OutputWindow = win32State->window;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.Flags = 0;

		uint32 debug = 0;
#if SOL_DEBUG_RENDERING
		debug = D3D11_CREATE_DEVICE_DEBUG;
#endif

		if (!FAILED(D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			debug,
			NULL, 0, // @NOTE: Pick the highest feature level
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&renderState.swapChain.swapChain,
			&renderState.deviceContext.device,
			&featureLevel,
			&renderState.deviceContext.context)))
		{
			SOLINFO("DX11 Swapchain and device created");

			CreateSwapChainBuffers();
			CreateAllRasterState();
			CreateAllDepthStencilState();
			CreateAllBlendState();
			EventSystem::Register((uint16)EventCodeEngine::WINDOW_RESIZED, 0, OnWindowResizeCallback);

			return true;
		}
		else
		{
			SOLFATAL("DX11 Could not create device,swapchain ");
		}

		return false;
	}

	void Renderer::Render(RenderPacket* renderPacket)
	{
		real32 windowWidth = (real32)Platform::GetWindowWidth();
		real32 windowHeight = (real32)Platform::GetWindowHeight();

		RenderCommand::ClearRenderTarget(renderState.swapChain.renderView, Vec4f(0.2f, 0.2f, 0.2f, 1.0f));
		RenderCommand::ClearDepthBuffer(renderState.swapChain.depthView);

		RenderCommand::SetRenderTargets(renderState.swapChain.renderView, renderState.swapChain.depthView);
		RenderCommand::SetTopology(Topology::Value::TRIANGLE_LIST);
		RenderCommand::SetViewportState(windowWidth, windowHeight);
		RenderCommand::SetDepthState(renderState.depthOffState);
		RenderCommand::SetRasterState(renderState.rasterNoFaceCullState);

		DeviceContext dc = renderState.deviceContext;
		DXCHECK(renderState.swapChain.swapChain->Present(1, 0));
	}

	void Renderer::Shutdown()
	{
		DXRELEASE(renderState.swapChain.depthView);
		DXRELEASE(renderState.swapChain.depthShaderView);
		DXRELEASE(renderState.swapChain.renderView);
		DXRELEASE(renderState.swapChain.depthTexture);
		DXRELEASE(renderState.swapChain.swapChain);
		DXRELEASE(renderState.deviceContext.context);
		DXRELEASE(renderState.deviceContext.device);
	}
}
#endif