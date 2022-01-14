#include "../SolarRenderer.h"
#include "core/SolarEvent.h"
#include "core/SolarLogging.h"
#if SOLAR_PLATFORM_WINDOWS && USE_DIRECTX11

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

	void* sol::Renderer::GetNativeDeviceContext()
	{
		return &renderState.deviceContext;
	}
#if SOL_DEBUG_RENDERING
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
#endif

	static void CreateSwapChainBuffers()
	{
		DeviceContext dc = renderState.deviceContext;
		HWND window = (HWND)Platform::GetNativeState();

		// @NOTE: Get back buffer
		ID3D11Resource* backBuffer = nullptr;
		DXCHECK(renderState.swapChain.swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backBuffer));
		DXCHECK(renderState.deviceContext.device->CreateRenderTargetView(backBuffer, nullptr, &renderState.swapChain.renderView));
		backBuffer->Release();

		RECT windowRect = {};
		GetClientRect(window, &windowRect);

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

		{
			D3D11_RASTERIZER_DESC rsDesc = {};
			rsDesc.FillMode = D3D11_FILL_WIREFRAME;
			rsDesc.CullMode = D3D11_CULL_NONE;
			rsDesc.FrontCounterClockwise = FALSE;
			rsDesc.DepthBias = 0;
			rsDesc.DepthBiasClamp = 1.0f;
			rsDesc.SlopeScaledDepthBias = 0.0f;
			rsDesc.DepthClipEnable = TRUE;
			rsDesc.MultisampleEnable = FALSE;
			rsDesc.AntialiasedLineEnable = FALSE;
			DXCHECK(dc.device->CreateRasterizerState(&rsDesc, &renderState.rasterNoFaceCullWireframe));
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

	static void CreateAllSamplerState()
	{
		renderState.pointRepeat = SamplerState::Create(TextureFilterMode::Value::POINT, TextureWrapMode::Value::REPEAT);
		renderState.bilinearRepeat = SamplerState::Create(TextureFilterMode::Value::BILINEAR, TextureWrapMode::Value::CLAMP_EDGE);
		renderState.trilinearRepeat = SamplerState::Create(TextureFilterMode::Value::TRILINEAR, TextureWrapMode::Value::REPEAT);
		renderState.shadowPFC = SamplerState::CreateShadowPFC();

		RenderCommand::SetSampler(renderState.pointRepeat, 0);
		RenderCommand::SetSampler(renderState.bilinearRepeat, 1);
		RenderCommand::SetSampler(renderState.trilinearRepeat, 2);
		RenderCommand::SetSampler(renderState.shadowPFC, 3);
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

		SOLINFO(String("Recreating swap chain").Add(resizeEvent->width).Add(":").Add(resizeEvent->height).GetCStr());

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
		HWND window = (HWND)Platform::GetNativeState();
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;


		uint32 deviceDebug = 0;
#if SOL_DEBUG_RENDERING
		deviceDebug = D3D11_CREATE_DEVICE_DEBUG;
#endif

		ID3D11Device* tempDevice = nullptr;
		ID3D11DeviceContext* tempContext = nullptr;
		auto hr = D3D11CreateDevice(
			NULL, D3D_DRIVER_TYPE_HARDWARE,
			NULL, deviceDebug,
			NULL, 0, D3D11_SDK_VERSION,
			&tempDevice, &featureLevel, &tempContext
		);

		if (SUCCEEDED(hr))
		{
			hr = tempDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&renderState.deviceContext.device);
			if (SUCCEEDED(hr))
			{
				hr = tempContext->QueryInterface(__uuidof(ID3D11DeviceContext), (void**)&renderState.deviceContext.context);
				if (SUCCEEDED(hr))
				{
					IDXGIFactory2* dxgiFactory = nullptr;
					hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
					if (SUCCEEDED(hr))
					{
						DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
						swapChainDesc.Width = 0;
						swapChainDesc.Height = 0;
						swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
						swapChainDesc.Stereo = FALSE;
						swapChainDesc.SampleDesc.Count = 1;
						swapChainDesc.SampleDesc.Quality = 0;
						swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
						swapChainDesc.BufferCount = 3;
						swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
						swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
						swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
						swapChainDesc.Flags = 0;

						hr = dxgiFactory->CreateSwapChainForHwnd(renderState.deviceContext.device, window,
							&swapChainDesc, NULL, NULL, &renderState.swapChain.swapChain);

						if (SUCCEEDED(hr))
						{
							CreateSwapChainBuffers();
							SOLINFO("DX11 Swapchain and device created");

							CreateAllRasterState();
							CreateAllDepthStencilState();
							CreateAllBlendState();
							CreateAllSamplerState();

							EventSystem::Register((uint16)EngineEvent::Value::WINDOW_RESIZED, 0, OnWindowResizeCallback);

							renderState.postProcessingProgram = ProgramInstance::CreateGraphics(*Resources::GetProgramResource("post_processing"));
							renderState.phongProgram = ProgramInstance::CreateGraphics(*Resources::GetProgramResource("phong"));
							{
								ProgramResource* res = Resources::GetProgramResource("phongKenney");
								res->vertexLayout = VertexLayoutType::Value::PNTC;
								renderState.phongKenneyProgram = ProgramInstance::CreateGraphics(*res);
							}

							renderState.modelConstBuffer = ShaderConstBuffer<ShaderConstBufferModel>::Create();
							renderState.viewConstBuffer = ShaderConstBuffer<ShaderConstBufferView>::Create();
							renderState.lightingConstBuffer = ShaderConstBuffer<ShaderConstBufferLightingInfo>::Create();
							renderState.uiConstBuffer = ShaderConstBuffer<ShaderConstBufferUIData>::Create();

							RenderCommand::SetShaderConstBuffer(&renderState.modelConstBuffer, ShaderStage::VERTEX, 0);
							RenderCommand::SetShaderConstBuffer(&renderState.viewConstBuffer, ShaderStage::VERTEX, 1);
							RenderCommand::SetShaderConstBuffer(&renderState.lightingConstBuffer, ShaderStage::PIXEL, 0);
							RenderCommand::SetShaderConstBuffer(&renderState.uiConstBuffer, ShaderStage::PIXEL, 4);

							RenderCommand::UploadShaderConstBuffer(&renderState.modelConstBuffer);
							RenderCommand::UploadShaderConstBuffer(&renderState.viewConstBuffer);
							RenderCommand::UploadShaderConstBuffer(&renderState.lightingConstBuffer);
							RenderCommand::UploadShaderConstBuffer(&renderState.uiConstBuffer);

							renderState.quad = StaticMesh::Create(ModelGenerator::CreateQuad(-1, 1, 2, 2, 0));
							renderState.cube = StaticMesh::Create(ModelGenerator::CreateBox(1, 1, 1, 1, VertexLayoutType::Value::PNT));

							ManagedArray<ModelResource> models = Resources::GetAllModelResources();
							for (uint32 i = 1; i < models.count; i++)
							{
								StaticMesh mesh = StaticMesh::Create(&models[i]);
								renderState.staticMeshes.Put(models[i].id, mesh);
							}

							ManagedArray<TextureResource> textures = Resources::GetAllTextureResources();
							for (uint32 i = 1; i < textures.count; i++)
							{
								TextureInstance texture = TextureInstance::Create(&textures[i]);
								renderState.textures.Put(textures[i].id, texture);
							}

							return true;
						}
						else
						{
							SOLFATAL("DX11 Could not create swapchain1, please make sure all drivers are up to date.")
						}
					}
					else
					{
						SOLFATAL("DX11 Could not create factory2, please make sure all drivers are up to date.")
					}
				}
				else
				{
					SOLFATAL("DX11 Could not create context1, please make sure all drivers are up to date.")
				}
			}
			else
			{
				SOLFATAL("DX11 Could not create device1, please make sure all drivers are up to date.")
			}
		}
		else
		{
			SOLFATAL("DX11 Could not create device, please make sure all drivers are up to date.")
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
		RenderCommand::SetDepthState(renderState.depthLessEqualState);
		RenderCommand::SetRasterState(renderState.rasterBackFaceCullingState);

		Mat4f view = renderPacket->viewMatrix;
		Mat4f proj = renderPacket->projectionMatrix;

		renderState.viewConstBuffer.data.view = view;
		renderState.viewConstBuffer.data.persp = proj;
		renderState.viewConstBuffer.data.screeenProjection = Mat4f(1);
		RenderCommand::UploadShaderConstBuffer(&renderState.viewConstBuffer);


		for (uint32 i = 0; i < renderPacket->renderEntries.count; i++)
		{
			RenderEntry* entry = &renderPacket->renderEntries[i];
			Mat4f m = entry->worldTransform.CalculateTransformMatrix();
			renderState.modelConstBuffer.data.model = m;
			renderState.modelConstBuffer.data.invM = Inverse(m);
			renderState.modelConstBuffer.data.mvp = m * view * proj;
			RenderCommand::UploadShaderConstBuffer(&renderState.modelConstBuffer);

#if 1
			//if (entry->material.albedoId.IsValid() && entry->material.modelId.IsValid())
			//{
			//	RenderCommand::SetTexture(*renderState.textures.Get(entry->material.albedoId), 0);
			//	RenderCommand::SetProgram(renderState.phongProgram);
			//	RenderCommand::DrawStaticMesh(*renderState.staticMeshes.Get(entry->material.modelId));
			//}

			RenderCommand::SetTexture(renderState.textures.GetValueSet()[4], 0);
			RenderCommand::SetProgram(renderState.phongProgram);
			RenderCommand::DrawStaticMesh(renderState.quad);

#else
			RenderCommand::SetTexture(renderState.textures.GetValueSet()[4], 0);
			if (entry->material.modelId.IsValid())
			{
				StaticMesh* mesh = renderState.staticMeshes.Get(entry->material.modelId);
				if (mesh->vertexLayout == VertexLayoutType::Value::PNT)
				{
					RenderCommand::SetProgram(renderState.phongProgram);
				}
				else if (mesh->vertexLayout == VertexLayoutType::Value::PNTC)
				{
					RenderCommand::SetProgram(renderState.phongKenneyProgram);
				}
				RenderCommand::DrawStaticMesh(*mesh);
			}
#endif
		}

		EventSystem::Fire((uint16)EngineEvent::Value::ON_RENDER_END, nullptr, {});
		DeviceContext dc = renderState.deviceContext;

		DXGI_PRESENT_PARAMETERS parameters = { 0 };
		DXCHECK(renderState.swapChain.swapChain->Present1(1, 0, &parameters));
	}

	void Renderer::Shutdown()
	{
		EventSystem::Fire((uint16)EngineEvent::Value::ON_RENDERER_SHUTDOWN, nullptr, {});

		RenderCommand::SetStaticMesh({});
		for (int32 i = 0; i < 10; i++) { RenderCommand::SetTexture({}, i); }

		StaticMesh::Release(&renderState.quad);
		StaticMesh::Release(&renderState.cube);

		ManagedArray<StaticMesh> meshes = renderState.staticMeshes.GetValueSet();
		for (uint32 i = 0; i < meshes.count; i++) { StaticMesh::Release(&meshes[i]); }
		meshes.Clear();

		ManagedArray<TextureInstance> textures = renderState.textures.GetValueSet();
		for (uint32 i = 0; i < textures.count; i++) { TextureInstance::Release(&textures[i]); }
		textures.Clear();

		ShaderConstBuffer<ShaderConstBufferModel>::Release(&renderState.modelConstBuffer);
		ShaderConstBuffer<ShaderConstBufferView>::Release(&renderState.viewConstBuffer);
		ShaderConstBuffer<ShaderConstBufferLightingInfo>::Release(&renderState.lightingConstBuffer);
		ShaderConstBuffer<ShaderConstBufferUIData>::Release(&renderState.uiConstBuffer);

		for (uint32 i = 0; i < ArrayCount(renderState.allSampleStates); i++) { SamplerState::Release(&renderState.allSampleStates[i]); }
		for (uint32 i = 0; i < ArrayCount(renderState.allRastersStates); i++) { DXRELEASE(renderState.allRastersStates[i]); }
		for (uint32 i = 0; i < ArrayCount(renderState.allDepthStates); i++) { DXRELEASE(renderState.allDepthStates[i]); }
		for (uint32 i = 0; i < ArrayCount(renderState.programs); i++) { ProgramInstance::Release(&renderState.programs[i]); }

		DXRELEASE(renderState.blendNormal);

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