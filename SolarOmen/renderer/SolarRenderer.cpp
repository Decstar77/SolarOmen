#include "SolarRenderer.h"
#include "../Debug.h"

namespace cm
{
	void LogDirectXDebugGetMessages(RenderDebug* debug)
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

	static void InitializeDirectXDebugLogging(RenderState* rs)
	{
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

	static void InitializeDirectXDebugDrawing(RenderState* rs, AssetState* as)
	{
		RenderDebug* rd = &rs->debug;
		DebugState* ds = GetDebugState();


		// Create vertex buffer
		D3D11_BUFFER_DESC vertex_desc = {};
		vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertex_desc.Usage = D3D11_USAGE_DYNAMIC;
		vertex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertex_desc.MiscFlags = 0;
		vertex_desc.ByteWidth = ds->vertex_size_bytes;
		vertex_desc.StructureByteStride = sizeof(real32) * ds->vertex_stride;

		D3D11_SUBRESOURCE_DATA vertex_res = {};
		vertex_res.pSysMem = ds->vertex_data;

		DXCHECK(rs->device->CreateBuffer(&vertex_desc, &vertex_res, &rd->vertex_buffer));
	}

	static void CreateDeviceAndSwapChain(RenderState* rs, PlatformState* ws)
	{
		LOGTODO("D3D11 is always using debug context");
		D3D_FEATURE_LEVEL feature_level;

		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.RefreshRate.Denominator = 0; //1000;
		sd.BufferDesc.RefreshRate.Numerator = 0;   // 60000;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1; // @NOTE This implies just the back buffer, ie we have two buffers
		sd.OutputWindow = (HWND)ws->window;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		DXCHECK(D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			D3D11_CREATE_DEVICE_DEBUG,
			NULL, 0, // @NOTE: Pick the highest feature level
			D3D11_SDK_VERSION,
			&sd,
			&rs->swapChain,
			&rs->device,
			&feature_level,
			&rs->context));

		ID3D11RenderTargetView* render_target = nullptr;
		ID3D11DepthStencilView* depth_target = nullptr;
		ID3D11ShaderResourceView* shaderView = nullptr;
		// @NOTE: Get back buffer
		{

			ID3D11Resource* back_buffer = nullptr;
			DXCHECK(rs->swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&back_buffer));
			DXCHECK(rs->device->CreateRenderTargetView(back_buffer, nullptr, &render_target));
			back_buffer->Release();
		}

		// @NOTE: Create back buffer depth texture 
		{
			RECT window_rect;
			GetClientRect((HWND)ws->window, &window_rect);

			ID3D11Texture2D* depth_texture = nullptr;
			D3D11_TEXTURE2D_DESC depth_ds = {};
			depth_ds.Width = (uint32)(window_rect.right - window_rect.left);
			depth_ds.Height = (uint32)(window_rect.bottom - window_rect.top);
			depth_ds.MipLevels = 1;
			depth_ds.ArraySize = 1;
			depth_ds.Format = DXGI_FORMAT_R32_TYPELESS;
			depth_ds.SampleDesc.Count = 1;
			depth_ds.SampleDesc.Quality = 0;
			depth_ds.Usage = D3D11_USAGE_DEFAULT;
			depth_ds.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

			DXINFO(rs->device->CreateTexture2D(&depth_ds, nullptr, &depth_texture));


			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
			depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
			depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_dsc.Texture2D.MipSlice = 0;
			DXCHECK(rs->device->CreateDepthStencilView(depth_texture, &depth_view_dsc, &depth_target));

			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_R32_FLOAT;
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MostDetailedMip = 0;
			view_desc.Texture2D.MipLevels = 1;

			DXCHECK(rs->device->CreateShaderResourceView(depth_texture, &view_desc, &shaderView));
		}

		// @NOTE: Save the render targets		
		rs->swapChainRenderTarget.depthTarget.guaranteeValid = true;
		rs->swapChainRenderTarget.depthTarget.depthView = depth_target;
		rs->swapChainRenderTarget.depthTarget.shaderView = shaderView;

		rs->swapChainRenderTarget.colourTarget0.guaranteeValid = true;
		rs->swapChainRenderTarget.colourTarget0.renderView = render_target;
	}

	static void SetRasterState(RenderState* rs, ID3D11RasterizerState* rasterState)
	{
		DXINFO(rs->context->RSSetState(rasterState));
	}

	static void CreateAllRasterState(RenderState* rs)
	{
		{
			D3D11_RASTERIZER_DESC rs_desc = {};
			rs_desc.FillMode = D3D11_FILL_SOLID;
			rs_desc.CullMode = D3D11_CULL_BACK;
			rs_desc.FrontCounterClockwise = FALSE;
			rs_desc.DepthBias = 0;
			rs_desc.DepthBiasClamp = 1.0f;
			rs_desc.SlopeScaledDepthBias = 0.0f;
			rs_desc.DepthClipEnable = TRUE;
			rs_desc.MultisampleEnable = FALSE;
			rs_desc.AntialiasedLineEnable = FALSE;

			DXCHECK(rs->device->CreateRasterizerState(&rs_desc, &rs->rasterNormal));
			SetRasterState(rs, rs->rasterNormal);
		}

		{
			D3D11_RASTERIZER_DESC rs_desc = {};
			rs_desc.FillMode = D3D11_FILL_SOLID;
			rs_desc.CullMode = D3D11_CULL_FRONT;
			rs_desc.FrontCounterClockwise = FALSE;
			rs_desc.DepthBias = 0;
			rs_desc.DepthBiasClamp = 1.0f;
			rs_desc.SlopeScaledDepthBias = 0.0f;
			rs_desc.DepthClipEnable = TRUE;
			rs_desc.MultisampleEnable = FALSE;
			rs_desc.AntialiasedLineEnable = FALSE;
			DXCHECK(rs->device->CreateRasterizerState(&rs_desc, &rs->rasterFrontFaceCullingState));
		}

		{
			D3D11_RASTERIZER_DESC rs_desc = {};
			rs_desc.FillMode = D3D11_FILL_SOLID;
			rs_desc.CullMode = D3D11_CULL_NONE;
			rs_desc.FrontCounterClockwise = FALSE;
			rs_desc.DepthBias = 0;
			rs_desc.DepthBiasClamp = 1.0f;
			rs_desc.SlopeScaledDepthBias = 0.0f;
			rs_desc.DepthClipEnable = TRUE;
			rs_desc.MultisampleEnable = FALSE;
			rs_desc.AntialiasedLineEnable = FALSE;
			DXCHECK(rs->device->CreateRasterizerState(&rs_desc, &rs->rasterNoFaceCullState));
		}
	}

	static void SetDepthState(RenderState* rs, ID3D11DepthStencilState* depthState)
	{
		DXINFO(rs->context->OMSetDepthStencilState(depthState, 1));
	}

	static void CreateAllDepthStencilState(RenderState* rs)
	{
		{
			D3D11_DEPTH_STENCIL_DESC ds = {};
			ds.DepthEnable = TRUE;
			ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			ds.DepthFunc = D3D11_COMPARISON_LESS;
			ds.StencilEnable = FALSE;

			DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->depthNormal));
			SetDepthState(rs, rs->depthNormal);
		}

		{
			D3D11_DEPTH_STENCIL_DESC ds = {};
			ds.DepthEnable = FALSE;
			ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			ds.DepthFunc = D3D11_COMPARISON_ALWAYS;
			ds.StencilEnable = FALSE;
			DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->depthOffState));
		}

		{
			D3D11_DEPTH_STENCIL_DESC ds = {};
			ds.DepthEnable = TRUE;
			ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			ds.StencilEnable = FALSE;
			DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->depthLessEqualState));
		}
	}

	static void CreateAllBlendState(RenderState* rs)
	{
		{
			D3D11_BLEND_DESC blend_desc = {};
			blend_desc.RenderTarget[0].BlendEnable = FALSE;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			DXCHECK(rs->device->CreateBlendState(&blend_desc, &rs->blendNormal));
			rs->context->OMSetBlendState(rs->blendNormal, nullptr, 0xffffffff);
		}
	}

	static void CreateAllShaders(RenderState* rs, AssetState* as)
	{
		ShaderInstance::CreateCompute(&as->shadersData[(int32)ShaderId::Value::DEPTH_REDUCTION]);
		ShaderInstance::CreateCompute(&as->shadersData[(int32)ShaderId::Value::DEPTH_REDUCTION_DOWN]);

		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::DEBUG_LINE], VertexShaderLayout::P);
		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::UNLIT], VertexShaderLayout::PNT);
		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::PHONG], VertexShaderLayout::PNT);
		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::BASIC_PBR], VertexShaderLayout::PNT);
		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::EQUIRECTANGULAR_TO_CUBEMAP], VertexShaderLayout::PNT);
		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::SKYBOX], VertexShaderLayout::PNT);
		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::IRRADIANCE_CONVOLUTION], VertexShaderLayout::PNT);
		ShaderInstance::CreateGraphics(&as->shadersData[(int32)ShaderId::Value::DEPTH_ONLY], VertexShaderLayout::PNT);
	}

	static MeshInstance CreateMeshInstance(RenderState* rs, real32* vertices, int32 vertexCount, uint32* indices, int32 indexCount)
	{
		uint32 vertex_stride_bytes = sizeof(real32) * 3 + sizeof(real32) * 3 + sizeof(real32) * 2;
		uint32 indices_stride_bytes = sizeof(uint32);

		D3D11_BUFFER_DESC vertex_desc = {};
		vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertex_desc.Usage = D3D11_USAGE_DEFAULT;
		vertex_desc.CPUAccessFlags = 0;
		vertex_desc.MiscFlags = 0;
		vertex_desc.ByteWidth = vertexCount * sizeof(real32);
		vertex_desc.StructureByteStride = vertex_stride_bytes;

		D3D11_SUBRESOURCE_DATA vertex_res = {};
		vertex_res.pSysMem = vertices;

		D3D11_BUFFER_DESC index_desc = {};
		index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		index_desc.Usage = D3D11_USAGE_DEFAULT;
		index_desc.CPUAccessFlags = 0;
		index_desc.MiscFlags = 0;
		index_desc.ByteWidth = indexCount * sizeof(uint32);
		index_desc.StructureByteStride = sizeof(uint32);

		D3D11_SUBRESOURCE_DATA index_res = {};
		index_res.pSysMem = indices;

		MeshInstance result = {  };
		DXCHECK(rs->device->CreateBuffer(&vertex_desc, &vertex_res, &result.vertexBuffer));
		DXCHECK(rs->device->CreateBuffer(&index_desc, &index_res, &result.indexBuffer));
		result.strideBytes = vertex_stride_bytes;
		result.indexCount = indexCount;

		return result;
	}

	static inline MeshInstance CreateMeshInstance(RenderState* rs, const MeshData& meshData)
	{
		MeshInstance result = CreateMeshInstance(rs, meshData.packedVertices,
			meshData.packedCount, meshData.indices, meshData.indicesCount);

		return result;
	}

	static ShaderConstBuffer CreateShaderBuffer(RenderState* rs, int32 sizeBytes)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.ByteWidth = sizeBytes;
		desc.StructureByteStride = 0;

		ShaderConstBuffer result = {};

		DXCHECK(rs->device->CreateBuffer(&desc, NULL, &result.buffer));
		result.sizeBytes = sizeBytes;
		result.copy_ptr = 0;
		Assert(ArrayCount(result.stagingBuffer) > sizeBytes, "Shader buffer staging buffer is not big enough");

		return result;
	}

	static void CreateAllShaderConstBuffers(RenderState* rs)
	{
		rs->vConstBuffers[0] = CreateShaderBuffer(rs, sizeof(Mat4f) * 3);
		rs->vConstBuffers[0].BindShaderBuffer(ShaderStage::VERTEX, 0);

		rs->vConstBuffers[1] = CreateShaderBuffer(rs, sizeof(Mat4f) * 2);
		rs->vConstBuffers[1].BindShaderBuffer(ShaderStage::VERTEX, 1);

		rs->pConstBuffers[0] = CreateShaderBuffer(rs, sizeof(Vec4f) * 65);
		rs->pConstBuffers[0].BindShaderBuffer(ShaderStage::PIXEL, 0);

		rs->pConstBuffers[3] = CreateShaderBuffer(rs, sizeof(Mat4f) * 5 + sizeof(Vec4f));
		rs->pConstBuffers[3].BindShaderBuffer(ShaderStage::PIXEL, 3);

		rs->cConstBuffers[0] = CreateShaderBuffer(rs, sizeof(Mat4f) + sizeof(Vec4f));
		rs->cConstBuffers[0].BindShaderBuffer(ShaderStage::COMPUTE, 0);
	}

	static void CreateScreenSpaceQuad(RenderState* rs)
	{
		real32 quad_data[] = {
			-1, 1, 0,	0, 0, -1,	0, 0,
			1, -1, 0,	0, 0, -1,	1, 1,
			-1, -1, 0,	0, 0, -1,	0, 1,
			1, 1, 0,	0, 0, -1,	1, 0
		};

		uint32 index_data[] = {
			0, 1, 2, 0, 3, 1
		};

		MeshInstance screenSpaceQuad = CreateMeshInstance(rs, quad_data, ArrayCount(quad_data), index_data, ArrayCount(index_data));
		screenSpaceQuad.id = ModelId::Value::SCREEN_SPACE_QUAD;

		rs->meshes[(int32)ModelId::Value::SCREEN_SPACE_QUAD] = screenSpaceQuad;
	}

	static void CreateAllMeshes(RenderState* rs, AssetState* as)
	{
		CreateScreenSpaceQuad(rs);

		// @NOTE: Staring at 2 here because, 0 is invalid and 1 is screen space quad
		for (int32 meshIndex = 2; meshIndex < as->meshCount; meshIndex++)
		{
			rs->meshes[meshIndex] = CreateMeshInstance(rs, as->meshesData[meshIndex]);
			rs->meshes[meshIndex].id = ModelId::ValueOf(meshIndex);
		}
	}



	static TextureInstance CreateTextureInstance(RenderState* rs, const TextureData& textureData)
	{
		Assert(!textureData.mips, "Todo mips");
		Assert(textureData.width > 0, "Width is zero");
		Assert(textureData.height > 0, "Width is zero");

		TextureInstance result = {};
		result.id = textureData.id;
		result.width = textureData.width;
		result.height = textureData.height;
		result.format = textureData.format;
		result.cpuFlags = textureData.cpuFlags;

		int32 bind_flags = 0;
		for (int32 i = 0; i < ArrayCount(textureData.usage); i++)
		{
			result.usage[i] = textureData.usage[i];
			bind_flags |= GetTextureUsageToD3DBindFlags(textureData.usage[i]);
		}

		// @NOTE: create the raw texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = textureData.width;
		desc.Height = textureData.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = GetTextureFormatToD3D(textureData.format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = textureData.cpuFlags == TextureCPUFlags::NONE ? D3D11_USAGE_DEFAULT : D3D11_USAGE_STAGING;
		desc.BindFlags = bind_flags;
		desc.CPUAccessFlags = GetTextureCPUFlagsToD3DFlags(textureData.cpuFlags);
		desc.MiscFlags = 0;

		if (textureData.pixels)
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = textureData.pixels;
			sd.SysMemPitch = textureData.width * GetTextureFormatElementSizeBytes(textureData.format) * GetTextureFormatElementCount(textureData.format);
			DXCHECK(rs->device->CreateTexture2D(&desc, &sd, &result.texture));
		}
		else
		{
			DXCHECK(rs->device->CreateTexture2D(&desc, NULL, &result.texture));
		}

		Assert(result.texture, "Could not create texture");

		if (result.texture && (bind_flags & D3D11_BIND_SHADER_RESOURCE))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			if (textureData.format == TextureFormat::R32_TYPELESS)
			{
				LOG("Warning making r32typeless format into a r32 float for shader resource");
				view_desc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else
			{
				view_desc.Format = desc.Format;
			}
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MostDetailedMip = 0;
			view_desc.Texture2D.MipLevels = 1;

			DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.shaderView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_UNORDERED_ACCESS))
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav = {};
			uav.Format = desc.Format;
			uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uav.Texture2D.MipSlice = 0;

			DXINFO(rs->device->CreateUnorderedAccessView(result.texture, &uav, &result.uavView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_DEPTH_STENCIL))
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
			if (textureData.format == TextureFormat::R32_TYPELESS)
			{
				LOG("Warning making r32typeless format into a d32 float for depth-stencil buffer");
				depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
			}
			else
			{
				depth_view_dsc.Format = desc.Format;
			}
			depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_dsc.Texture2D.MipSlice = 0;
			DXCHECK(rs->device->CreateDepthStencilView(result.texture,
				&depth_view_dsc, &result.depthView));
		}

		if (result.texture && (bind_flags & D3D11_BIND_RENDER_TARGET))
		{
			D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
			render_target_desc.Format = desc.Format;
			render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			render_target_desc.Texture2D.MipSlice = 0;

			DXCHECK(rs->device->CreateRenderTargetView(result.texture,
				&render_target_desc, &result.renderView));
		}

		return result;
	}

	static TextureArrayInstance CreateTextureArrayInstance(RenderState* rs, const TextureData& textureData, int32 count)
	{
		TextureArrayInstance result = {};
		result.id = textureData.id;
		result.width = textureData.width;
		result.height = textureData.height;
		result.format = textureData.format;
		result.count = count;

		int32 bind_flags = 0;
		for (int32 i = 0; i < ArrayCount(textureData.usage); i++)
		{
			result.usage[i] = textureData.usage[i];
			bind_flags |= GetTextureUsageToD3DBindFlags(textureData.usage[i]);
		}

		// @NOTE: create the raw texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = textureData.width;
		desc.Height = textureData.height;
		desc.MipLevels = 1;
		desc.ArraySize = count;
		desc.Format = DXGI_FORMAT_R16_TYPELESS;// GetTextureFormatToD3D(textureData.format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = bind_flags;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		DXCHECK(rs->device->CreateTexture2D(&desc, NULL, &result.texture));
		Assert(result.texture, "Could not create texture 2d array instance");

		if (result.texture)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_R16_UNORM;
			//view_desc.Format = DXGI_FORMAT_R32_FLOAT;
			view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
			view_desc.Texture2DArray.ArraySize = count;
			view_desc.Texture2DArray.MostDetailedMip = 0;
			view_desc.Texture2DArray.MipLevels = 1;
			view_desc.Texture2DArray.FirstArraySlice = 0;

			DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.shaderView));

			for (int32 i = 0; i < count; i++)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
				depth_view_dsc.Format = DXGI_FORMAT_D16_UNORM;
				//depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
				depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				depth_view_dsc.Texture2DArray.MipSlice = 0;
				depth_view_dsc.Texture2DArray.ArraySize = 1;
				depth_view_dsc.Texture2DArray.FirstArraySlice = i;

				DXCHECK(rs->device->CreateDepthStencilView(result.texture,
					&depth_view_dsc, &result.depthViews[i]));
			}
		}

		return result;
	}

	static CubeMapInstance CreateCubeMapInstance(RenderState* rs, int32 width, int32 height)
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 6;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		CubeMapInstance result = {};
		DXCHECK(rs->device->CreateTexture2D(&desc, NULL, &result.texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.Format = desc.Format;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		view_desc.Texture2D.MostDetailedMip = 0;
		view_desc.Texture2D.MipLevels = 1;

		DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.shaderView));

		D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
		render_target_desc.Format = desc.Format;
		render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		render_target_desc.Texture2DArray.MipSlice = 0;
		render_target_desc.Texture2DArray.ArraySize = 1;

		for (int32 i = 0; i < 6; i++)
		{
			render_target_desc.Texture2DArray.FirstArraySlice = i;
			DXCHECK(rs->device->CreateRenderTargetView(result.texture,
				&render_target_desc, &result.renderFaces[i]));
		}

		return result;
	}

	static void CreateAllTexture(RenderState* rs, AssetState* as)
	{
		for (int32 textureIndex = 1; textureIndex < as->textureCount; textureIndex++)
		{
			rs->textures[textureIndex] = CreateTextureInstance(rs, as->texturesData[textureIndex]);
			rs->textures[textureIndex].id = TextureId::ValueOf(textureIndex);
		}
	}

	static void CreateAllSkybox(RenderState* rs, AssetState* as)
	{
		//TextureData textureData = {};
		//SkyboxAsset skyboxAsset = as->skyboxes[1];

		//textureData.width = skyboxAsset.width;
		//textureData.height = skyboxAsset.height;
		//textureData.format = skyboxAsset.format;
		//textureData.pixels = skyboxAsset.pixels;
		//textureData.usage[0] = skyboxAsset.usage[0];

		//rs->testTex = CreateTextureInstance(rs, textureData);

		//for (int32 skyboxIndex = 1; skyboxIndex < as->skyboxCount; skyboxIndex++)
		//{

		//}
	}


	static void CreateAllSamplers(RenderState* rs)
	{
		rs->pointRepeat = SamplerInstance::Create(rs, TextureFilterMode::POINT, TextureWrapMode::REPEAT, 0);
		rs->bilinearRepeat = SamplerInstance::Create(rs, TextureFilterMode::BILINEAR, TextureWrapMode::CLAMP_EDGE, 1);
		rs->trilinearRepeat = SamplerInstance::Create(rs, TextureFilterMode::TRILINEAR, TextureWrapMode::REPEAT, 2);

		rs->shadowPFC = SamplerInstance::CreateShadowPFC(rs, 3);
	}

	static inline void SetViewport(RenderState* rs, real32 width, real32 height, real32 minDepth = 0.0f,
		real32 maxDepth = 1.0f, real32 topLeftX = 0.0f, real32 topLeftY = 0.0f)
	{
		D3D11_VIEWPORT viewport = {};

		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = minDepth;
		viewport.MaxDepth = maxDepth;
		viewport.TopLeftX = topLeftX;
		viewport.TopLeftY = topLeftY;

		DXINFO(rs->context->RSSetViewports(1, &viewport));
	}

	static void RenderScreenSpaceQuad(RenderState* rs)
	{
		rs->meshes[(int32)ModelId::Value::SCREEN_SPACE_QUAD].Bind(rs);
		rs->meshes[(int32)ModelId::Value::SCREEN_SPACE_QUAD].DrawIndexed(rs);
	}

	static void RenderCube(RenderState* rs)
	{
		rs->meshes[(int32)ModelId::Value::CUBE].Bind(rs);
		rs->meshes[(int32)ModelId::Value::CUBE].DrawIndexed(rs);
	}

	// @TODO: Parameterize
	static void ConvertHDRToCubeMap(RenderState* rs, AssetState* as)
	{

	}

	uint32 DispatchSize(uint32 groupSize, uint32 numElements)
	{
		uint32 dispatchSize = numElements / groupSize;
		dispatchSize += numElements % groupSize > 0 ? 1 : 0;
		return dispatchSize;
	}

	static void ApplySettings(RenderState* rs, AssetState* as, PlatformState* ws)
	{

		// @NOTE: Shadow initialization
		{
			// @NOTE: Create the shadow textures
			//int32 shadowRes = 4096;
			//int32 shadowRes = 2048;
			int32 shadowRes = RenderingSettings::shadowQuality.GetResolution();
			TextureData texData = {};
			texData.id = TextureId::Value::INTERNAL;
			texData.width = shadowRes;
			texData.height = shadowRes;
			texData.format = TextureFormat::R32_TYPELESS;
			texData.usage[0] = TextureUsage::DEPTH_SCENCIL_BUFFER;
			texData.usage[1] = TextureUsage::SHADER_RESOURCE;

			rs->shadowCascades = CreateTextureArrayInstance(rs, texData, 4);

			// @NOTE: Create reduction textures
			uint32 w = ws->client_width;
			uint32 h = ws->client_height;
			while (w > 1 || h > 1)
			{
				w = DispatchSize(16, w);
				h = DispatchSize(16, h);
				texData = {};
				texData.id = TextureId::Value::INTERNAL;
				texData.width = w;
				texData.height = h;
				texData.format = TextureFormat::R16G16_UNORM;
				texData.usage[0] = TextureUsage::COMPUTER_SHADER_RESOURCE;
				texData.usage[1] = TextureUsage::SHADER_RESOURCE;

				int32 index = rs->reductionTargetCount;
				rs->reductionTargets[index] = CreateTextureInstance(rs, texData);
				rs->reductionTargetCount++;
			}

			texData = {};
			texData.id = TextureId::Value::INTERNAL;
			texData.width = 1;
			texData.height = 1;
			texData.format = TextureFormat::R16G16_UNORM;
			texData.cpuFlags = TextureCPUFlags::READ;

			rs->reductionStagingTex = CreateTextureInstance(rs, texData);
		}

		int32 resolution = 512;
		{
			rs->environmentMap = CreateCubeMapInstance(rs, resolution, resolution);

			SkyboxAsset skyboxAsset = as->skyboxes[2];
			TextureData texData = {};
			texData.width = skyboxAsset.width;
			texData.height = skyboxAsset.height;
			texData.pixels = skyboxAsset.pixels;
			texData.format = skyboxAsset.format;
			texData.usage[0] = skyboxAsset.usage[0];

			rs->eqiTexture = CreateTextureInstance(rs, texData);
			rs->eqiTexture.guaranteeValid = true;
		}

		SetViewport(rs, (real32)resolution, (real32)resolution);
		rs->shaders[(int32)ShaderId::Value::EQUIRECTANGULAR_TO_CUBEMAP].Bind();

		ShaderConstBuffer* vC0 = &rs->vConstBuffers[0];
		ShaderConstBuffer* vC1 = &rs->vConstBuffers[1];
		vC0->BindShaderBuffer(ShaderStage::VERTEX, 0);
		vC1->BindShaderBuffer(ShaderStage::VERTEX, 1);

		vC0->CopyMat4fIntoShaderBuffer(Mat4f(1), false);
		vC0->CopyMat4fIntoShaderBuffer(Mat4f(1), false);
		vC0->CopyMat4fIntoShaderBuffer(Mat4f(1), false);
		vC0->CommitChanges();

		rs->eqiTexture.Bind(rs, ShaderStage::PIXEL, 0);

		SetRasterState(rs, rs->rasterNoFaceCullState);
		SetDepthState(rs, rs->depthOffState);

		Mat4f views[6] =
		{
			Inverse(LookAtLH(Vec3f(0), Vec3f(1, 0, 0), Vec3f(0, 1, 0))),
			Inverse(LookAtLH(Vec3f(0), Vec3f(-1, 0, 0), Vec3f(0, 1, 0))),


			Inverse(LookAtLH(Vec3f(0), Vec3f(0, 1, 0), Vec3f(0, 0, -1))),
			Inverse(LookAtLH(Vec3f(0), Vec3f(0, -1, 0), Vec3f(0, 0, 1))),

			Inverse(LookAtLH(Vec3f(0), Vec3f(0, 0, 1), Vec3f(0, 1, 0))),
			Inverse(LookAtLH(Vec3f(0), Vec3f(0, 0, -1), Vec3f(0, 1, 0)))
		};

		Mat4f proj = PerspectiveLH(DegToRad(90.0f), 1.0f, 0.1f, 10.0f);
		RenderTarget rt = { };
		for (int32 i = 0; i < 6; i++)
		{
			vC1->CopyMat4fIntoShaderBuffer(proj, false);
			vC1->CopyMat4fIntoShaderBuffer(views[i], false);
			vC1->CommitChanges();

			rt.colourTarget0.guaranteeValid = true;
			rt.colourTarget0.renderView = rs->environmentMap.renderFaces[i];

			rt.Bind(rs);
			rt.Clear(rs, Vec4f(0, 1, 0, 1));

			RenderCube(rs);
		}

		rt.Unbind(rs);

		int32 envRes = 32;

		rs->shaders[(int32)ShaderId::Value::IRRADIANCE_CONVOLUTION].Bind();
		rs->irradianceMap = CreateCubeMapInstance(rs, envRes, envRes);

		rs->environmentMap.Bind(rs, ShaderStage::PIXEL, 5);

		SetViewport(rs, (real32)envRes, (real32)envRes);

		for (int32 i = 0; i < 6; i++)
		{
			vC1->CopyMat4fIntoShaderBuffer(proj, false);
			vC1->CopyMat4fIntoShaderBuffer(views[i], false);
			vC1->CommitChanges();

			rt.colourTarget0.guaranteeValid = true;
			rt.colourTarget0.renderView = rs->irradianceMap.renderFaces[i];

			rt.Bind(rs);
			rt.Clear(rs, Vec4f(0, 1, 0, 1));

			RenderCube(rs);
		}

		SetRasterState(rs, rs->rasterNormal);
		SetDepthState(rs, rs->depthNormal);
	}

	void InitializeRenderState(RenderState* rs, AssetState* as, PlatformState* ws)
	{
		RenderState::GlobalRenderState = rs;

		InitializeDirectXDebugLogging(rs);
		CreateDeviceAndSwapChain(rs, ws);
		CreateAllRasterState(rs);
		CreateAllDepthStencilState(rs);
		CreateAllBlendState(rs);
		CreateAllSamplers(rs);

		InitializeDirectXDebugDrawing(rs, as);
		CreateAllShaders(rs, as);
		CreateAllShaderConstBuffers(rs);
		CreateAllMeshes(rs, as);
		CreateAllTexture(rs, as);
		CreateAllSkybox(rs, as);

		DXINFO(rs->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		ApplySettings(rs, as, ws);
	}

	void ShutdownRenderState(RenderState* rs)
	{
		// @TODO: Textures
		// @TODO: Shaders
		// @TODO: Const buffers
		// @TODO: Context + device + swapchain
		// @TODO: Samplers 
		// @TODO: Debug stuff ?

		for (int32 i = 0; i < ArrayCount(rs->rasterStates); i++)
		{
			DXRELEASE(rs->rasterStates[i]);
		}

		for (int32 i = 0; i < ArrayCount(rs->depthStates); i++)
		{
			DXRELEASE(rs->depthStates[i]);
		}

		for (int32 i = 0; i < ArrayCount(rs->blendStates); i++)
		{
			DXRELEASE(rs->blendStates[i]);
		}
	}

	inline Vec4f RGBToSRGB(int32 r, int32 g, int32 b, int32 a)
	{
		return Vec4f((real32)r, (real32)g, (real32)b, (real32)a) / Vec4f(255.0f);
	}

	static void UpdateSceneConstBuffers(RenderState* rs, EntityRenderGroup* renderGroup)
	{
		ShaderConstBuffer* vC0 = &rs->vConstBuffers[0];
		ShaderConstBuffer* vC1 = &rs->vConstBuffers[1];
		ShaderConstBuffer* pC0 = &rs->pConstBuffers[0];

		pC0->CopyVec3fIntoShaderBuffer(renderGroup->mainCamera.transform.position);
		pC0->CopyVec4iIntoShaderBuffer(Vec4i(0, 0, renderGroup->pointLightCount, 0));

		pC0->CopyVec3fIntoShaderBuffer(Vec3f(renderGroup->mainDirectionalLight.transform.GetBasis().forward));
		pC0->CopyVec3fIntoShaderBuffer(Vec3f(renderGroup->mainDirectionalLight.lightComp.colour * renderGroup->mainDirectionalLight.lightComp.intensity));

		pC0->CopyVec3fIntoShaderBuffer(Vec3f(0));
		pC0->CopyVec3fIntoShaderBuffer(Vec3f(0));

		for (int32 i = 0; i < MAX_SPOT_LIGHT_COUNT; i++)
		{
			pC0->CopyVec3fIntoShaderBuffer(Vec3f(0));
			pC0->CopyVec3fIntoShaderBuffer(Vec3f(0));
			pC0->CopyVec3fIntoShaderBuffer(Vec3f(0));
		}

		for (int32 i = 0; i < MAX_POINT_LIGHT_COUNT; i++)
		{
			Entity* entity = &renderGroup->pointLights[i];

			Transform world = entity->transform;
			pC0->CopyVec3fIntoShaderBuffer(world.position);
			pC0->CopyVec3fIntoShaderBuffer(Vec3f(1.0f) * 10.0f);
		}
		pC0->CommitChanges();


		vC0->BindShaderBuffer(ShaderStage::VERTEX, 0);
		vC1->BindShaderBuffer(ShaderStage::VERTEX, 1);
		pC0->BindShaderBuffer(ShaderStage::PIXEL, 0);

		Mat4f v = renderGroup->mainCamera.GetViewMatrix();
		Mat4f p = renderGroup->mainCamera.GetProjectionMatrix();
		vC1->CopyMat4fIntoShaderBuffer(p, false);
		vC1->CopyMat4fIntoShaderBuffer(v, false);
		vC1->CommitChanges();
	}

	void DEBUGRenderAndFlushDebugDraws(RenderState* rs)
	{
		RenderDebug* rd = &rs->debug;
		DebugState* ds = GetDebugState();

		D3D11_MAPPED_SUBRESOURCE resource = {};
		DXCHECK(rs->context->Map(rd->vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));

		memcpy(resource.pData, ds->vertex_data, ds->vertex_size_bytes);
		DXINFO(rs->context->Unmap(rd->vertex_buffer, 0));

		uint32 offset = 0;
		uint32 vertex_stride_bytes = ds->vertex_stride * sizeof(real32);
		DXINFO(rs->context->IASetVertexBuffers(0, 1, &rd->vertex_buffer, &vertex_stride_bytes, &offset));

		//////////////////////////////////
		//////////////////////////////////

		rs->shaders[(uint32)ShaderId::Value::DEBUG_LINE].Bind();

		//////////////////////////////////
		//////////////////////////////////

		DXINFO(rs->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST));

		DXINFO(rs->context->Draw(ds->next_vertex_index, 0));

		DXINFO(rs->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		//////////////////////////////////
		//////////////////////////////////

		ZeroMemory(ds->vertex_data, ds->vertex_size_bytes);
		ds->next_vertex_index = 0;
	}

	static void BindRenderTargets(RenderState* rs,
		ID3D11RenderTargetView* colour0,
		ID3D11RenderTargetView* colour1,
		ID3D11RenderTargetView* colour2,
		ID3D11RenderTargetView* colour3,
		ID3D11DepthStencilView* depth)
	{
		ID3D11RenderTargetView* views[4] = { nullptr, nullptr, nullptr, nullptr };

		views[0] = colour0;
		views[1] = colour1;
		views[2] = colour2;
		views[3] = colour3;

		DXINFO(rs->context->OMSetRenderTargets(4, views, depth));
	}

	static void ClearRenderTarget(RenderState* rs,
		ID3D11RenderTargetView* target, const Vec4f& colour)
	{
		DXINFO(rs->context->ClearRenderTargetView(target, colour.ptr));
	}

	static void ClearDepthTarget(RenderState* rs, ID3D11DepthStencilView* depth)
	{
		Assert(depth, "No depth view");
		DXINFO(rs->context->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1.0f, 0))
	}

	static void ClearDepthTarget(RenderState* rs, const TextureInstance& depth)
	{
		Assert(depth.depthView, "No depth view !");
		DXINFO(rs->context->ClearDepthStencilView(depth.depthView, D3D11_CLEAR_DEPTH, 1.0f, 0))
	}

	static void BindUAV(RenderState* rs, int32 register_, ID3D11UnorderedAccessView* uavView)
	{
		ID3D11UnorderedAccessView* views = { uavView };
		DXINFO(rs->context->CSSetUnorderedAccessViews(register_, 1, &views, nullptr));
	}

	static Mat4f CalculateLightVP(Transform lightTransform, const Mat4f& cameraProjection, const Mat4f& cameraView)
	{
		Vec3f lightForward = lightTransform.GetBasis().forward;

		//Frustrum frustrum = CreateFrustrum(lightProj, lightView);
		//Frustrum frustrum = CreateFrustrum(p, v);

		Frustrum frustrum = CreateFrustrum(cameraProjection, cameraView);
		FrustrumCorners corners = CalculateFrustrumCorners(frustrum);
		Vec3f center = corners.GetCenter();

		lightTransform.position = center - lightForward;
		lightTransform.LookAtLH(center);
		Mat4f lightView = Inverse(lightTransform.CalculateTransformMatrix());


#define STABLE_CASCADES 1
#if !STABLE_CASCADES 
		Vec3f min = Vec3f(REAL_MAX);
		Vec3f max = Vec3f(REAL_MIN);
		for (int32 i = 0; i < ArrayCount(corners.points); i++)
		{
			Vec3f lp = Vec3f(Vec4f(corners.points[i], 1.0f) * lightView);

			min = Min(lp, min);
			max = Max(lp, max);
		}

		//min = 1.1f * min;
		//max = 1.1f * max;

		Mat4f lightProj = OrthographicLH(min.x, max.x, max.y, min.y, min.z - 10.0f, max.z);
#else 
		Vec3f min = Vec3f(REAL_MAX);
		Vec3f max = Vec3f(REAL_MIN);

		real32 sphereRad = 0.0f;

		for (int32 i = 0; i < ArrayCount(corners.points); i++)
		{
			real32 dist = Mag(corners.points[i] - center);
			sphereRad = Max(sphereRad, dist);
		}
		sphereRad = std::ceil(sphereRad * 16.0f) / 16.0f;

		max = Vec3f(sphereRad);
		min = -1.0f * max;

		lightTransform.position = center - (lightForward * -min.z);
		lightTransform.LookAtLH(center);
		lightView = Inverse(lightTransform.CalculateTransformMatrix());

		Vec3f cascadeExtents = max - min;
		Mat4f lightProj = OrthographicLH(min.x, max.x, max.y, min.y, -20.1f, cascadeExtents.z);

		real32 shadowMapResolution = (real32)RenderingSettings::shadowQuality.GetResolution();

		Mat4f shadowMatrix = lightView * lightProj;
		Vec4f shadowOrigin = Vec4f(0, 0, 0, 1);
		shadowOrigin = shadowOrigin * shadowMatrix;
		shadowOrigin = shadowOrigin * (shadowMapResolution / 2.0f);

		Vec4f roundedOrigin = shadowOrigin;
		roundedOrigin.x = std::roundf(roundedOrigin.x);
		roundedOrigin.y = std::roundf(roundedOrigin.y);
		roundedOrigin.z = std::roundf(roundedOrigin.z);
		roundedOrigin.w = std::roundf(roundedOrigin.w);

		Vec4f roundOffset = roundedOrigin - shadowOrigin;
		roundOffset = roundOffset * 2.0f / shadowMapResolution;
		roundOffset.z = 0.0;
		roundOffset.w = 0.0;

		lightProj.row3 = lightProj.row3 + roundOffset;
#endif
		//Mat4f lightView = Inverse(renderGroup->mainDirectionalLight.transform.CalculateTransformMatrix());
		//Mat4f lightProj = OrthographicLH(-10.0f, 10.0f, 10.0f, -10.0f, 0.5f, 10.0f);


		DEBUGDrawFrustum(CreateFrustrum(lightProj, lightView));
		DEBUGDrawFrustum(frustrum);

		return lightView * lightProj;
	}

	void RenderGame(RenderState* rs, AssetState* as, EntityRenderGroup* renderGroup, PlatformState* ws, Input* input)
	{
		UpdateSceneConstBuffers(rs, renderGroup);
		Mat4f v = renderGroup->mainCamera.GetViewMatrix();
		Mat4f p = renderGroup->mainCamera.GetProjectionMatrix();

		real32 reductionDepthMin = 0.0f;
		real32 reductionDepthMax = 1.0f;

#if 0
		// @PASS: Depth pre-pass
		{
			ShaderInstance* shader = &rs->shaders[(int32)ShaderId::Value::DEPTH_ONLY];
			DXINFO(rs->context->VSSetShader(shader->vs, nullptr, 0));
			DXINFO(rs->context->PSSetShader(NULL, nullptr, 0));
			DXINFO(rs->context->IASetInputLayout(shader->layout));

			SetViewport(rs, (real32)ws->client_width, (real32)ws->client_height);

			ClearRenderTarget(rs, rs->swapChainRenderTarget.colourTarget0.renderView, RGBToSRGB(5, 5, 5, 255));
			ClearDepthTarget(rs, rs->swapChainRenderTarget.depthTarget);

			BindRenderTargets(rs, 0, 0, 0, 0, rs->swapChainRenderTarget.depthTarget.depthView);

			Mat4f DEBUGV = renderGroup->playerCamera.GetViewMatrix();
			Mat4f DEBUGP = renderGroup->playerCamera.GetProjectionMatrix();

			for (int32 i = 0; i < renderGroup->opaqueEntityCount; i++)
			{
				OpaqueRenderEntry* entry = &renderGroup->opaqueRenderEntries[i];

				Mat4f m = entry->transform.CalculateTransformMatrix();
				ShaderConstBuffer* vC0 = &rs->vConstBuffers[0];
				vC0->CopyMat4fIntoShaderBuffer(m * DEBUGV * DEBUGP, false);
				//vC0->CopyMat4fIntoShaderBuffer(m * v * p, false);
				vC0->CopyMat4fIntoShaderBuffer(m, false);
				vC0->CopyMat4fIntoShaderBuffer(Inverse(m), false);
				vC0->CommitChanges();

				Assert(entry->renderComp.modelId.IsValid(), "Invalid model id");
				rs->meshes[entry->renderComp.modelId].Bind(rs);
				rs->meshes[entry->renderComp.modelId].DrawIndexed(rs);
			}
		}


		// @PASS: Depth reduction

		{
			rs->shaders[(int32)ShaderId::Value::DEPTH_REDUCTION].Bind();

			ShaderConstBuffer* cC0 = &rs->cConstBuffers[0];
			cC0->CopyMat4fIntoShaderBuffer(renderGroup->playerCamera.GetProjectionMatrix(), false);
			cC0->CopyVec4fIntoShaderBuffer(Vec4f(renderGroup->playerCamera.near_, renderGroup->playerCamera.far_, 0.0f, 0.0f));
			cC0->CommitChanges();

			BindRenderTargets(rs, 0, 0, 0, 0, 0);

			rs->swapChainRenderTarget.depthTarget.Bind(rs, ShaderStage::COMPUTE, 0);
			BindUAV(rs, 0, rs->reductionTargets[0].uavView);

			uint32 dispatchX = rs->reductionTargets[0].width;
			uint32 dispatchY = rs->reductionTargets[0].height;
			DXINFO(rs->context->Dispatch(dispatchX, dispatchY, 1));

			rs->swapChainRenderTarget.depthTarget.Unbind(rs);
			BindUAV(rs, 0, nullptr);

			rs->shaders[(int32)ShaderId::Value::DEPTH_REDUCTION_DOWN].Bind();

			for (uint32 i = 1; i < rs->reductionTargetCount; i++)
			{
				BindUAV(rs, 0, rs->reductionTargets[i].uavView);
				rs->reductionTargets[i - 1].Bind(rs, ShaderStage::COMPUTE, 0);

				dispatchX = rs->reductionTargets[i].width;
				dispatchY = rs->reductionTargets[i].height;
				DXINFO(rs->context->Dispatch(dispatchX, dispatchY, 1));

				rs->swapChainRenderTarget.depthTarget.Unbind(rs);
				BindUAV(rs, 0, nullptr);
			}

			DXINFO(rs->context->CopyResource(rs->reductionStagingTex.texture,
				rs->reductionTargets[rs->reductionTargetCount - 1].texture));

			D3D11_MAPPED_SUBRESOURCE mapped;
			DXCHECK(rs->context->Map(rs->reductionStagingTex.texture, 0, D3D11_MAP_READ, 0, &mapped));
			const uint16* texData = reinterpret_cast<uint16*>(mapped.pData);
			reductionDepthMin = texData[0] / static_cast<float>(0xffff);
			reductionDepthMax = texData[1] / static_cast<float>(0xffff);

			rs->context->Unmap(rs->reductionStagingTex.texture, 0);
		}
#endif

		{
			// @NOTE: Unbind the texture array from the shader resource pool
			rs->shadowCascades.Unbind(rs);

			ShaderInstance* shader = &rs->shaders[(int32)ShaderId::Value::DEPTH_ONLY];
			DXINFO(rs->context->VSSetShader(shader->vs, nullptr, 0));
			DXINFO(rs->context->PSSetShader(NULL, nullptr, 0));
			DXINFO(rs->context->IASetInputLayout(shader->layout));

			SetDepthState(rs, rs->depthNormal);
			SetRasterState(rs, rs->rasterNormal);
			SetViewport(rs, (real32)rs->shadowCascades.width, (real32)rs->shadowCascades.height);

			const real32 minDistance = reductionDepthMin;
			const real32 maxDistance = reductionDepthMax;

			real32 cascadeSplits[4] = {
				minDistance + 0.05f * maxDistance,
				minDistance + 0.15f * maxDistance,
				minDistance + 0.5f * maxDistance,
				minDistance + 1.0f * maxDistance
			};

			real32 cascades[5] = {
				renderGroup->playerCamera.near_ ,
				renderGroup->playerCamera.far_ * cascadeSplits[0],
				renderGroup->playerCamera.far_ * cascadeSplits[1],
				renderGroup->playerCamera.far_ * cascadeSplits[2],
				renderGroup->playerCamera.far_ * cascadeSplits[3],
			};

			Mat4f lightVPs[4] = {};
			Mat4f playerView = renderGroup->playerCamera.GetViewMatrix();

			for (int32 shadowCascadeIndex = 0; shadowCascadeIndex < 4; shadowCascadeIndex++)
			{
				ClearDepthTarget(rs, rs->shadowCascades.depthViews[shadowCascadeIndex]);
				BindRenderTargets(rs, 0, 0, 0, 0, rs->shadowCascades.depthViews[shadowCascadeIndex]);

				Mat4f projection = PerspectiveLH(renderGroup->playerCamera.yfov, renderGroup->playerCamera.aspect,
					cascades[shadowCascadeIndex],
					cascades[shadowCascadeIndex + 1]);

				Mat4f lightVP = CalculateLightVP(renderGroup->mainDirectionalLight.transform,
					projection, playerView);

				lightVPs[shadowCascadeIndex] = lightVP;

				for (int32 i = 0; i < renderGroup->opaqueEntityCount; i++)
				{
					OpaqueRenderEntry* entry = &renderGroup->opaqueRenderEntries[i];
					if (entry->renderComp.HasFlagSet(RenderFlag::NO_CAST_SHADOW))
						continue;

					Mat4f m = entry->transform.CalculateTransformMatrix();
					ShaderConstBuffer* vC0 = &rs->vConstBuffers[0];
					vC0->CopyMat4fIntoShaderBuffer(m * lightVP, false);
					//vC0->CopyMat4fIntoShaderBuffer(m * v * p, false);
					vC0->CopyMat4fIntoShaderBuffer(m, false);
					vC0->CopyMat4fIntoShaderBuffer(Inverse(m), false);
					vC0->CommitChanges();

					Assert(entry->renderComp.modelId.IsValid(), "Invalid model id");
					rs->meshes[entry->renderComp.modelId].Bind(rs);
					rs->meshes[entry->renderComp.modelId].DrawIndexed(rs);
				}
			}

			ShaderConstBuffer* pC3 = &rs->pConstBuffers[3];
			pC3->CopyMat4fIntoShaderBuffer(playerView, false);
			pC3->CopyMat4fIntoShaderBuffer(lightVPs[0], false);
			pC3->CopyMat4fIntoShaderBuffer(lightVPs[1], false);
			pC3->CopyMat4fIntoShaderBuffer(lightVPs[2], false);
			pC3->CopyMat4fIntoShaderBuffer(lightVPs[3], false);
			pC3->CopyVec4fIntoShaderBuffer(Vec4f(cascades[1], cascades[2], cascades[3], cascades[4]));
			pC3->CommitChanges();
		}

		SetViewport(rs, (real32)ws->client_width, (real32)ws->client_height);


		// @TODO: Remove this, the pre depth does this !!!
		ClearRenderTarget(rs, rs->swapChainRenderTarget.colourTarget0.renderView, RGBToSRGB(5, 5, 5, 255));
		ClearDepthTarget(rs, rs->swapChainRenderTarget.depthTarget);

		BindRenderTargets(rs,
			rs->swapChainRenderTarget.colourTarget0.renderView,
			nullptr, nullptr, nullptr,
			rs->swapChainRenderTarget.depthTarget.depthView);

		rs->shadowCascades.Bind(rs, ShaderStage::PIXEL, 6);

		SetRasterState(rs, rs->rasterNoFaceCullState);
		SetDepthState(rs, rs->depthOffState);
		rs->shaders[(int32)ShaderId::Value::SKYBOX].Bind();

		static bool temp = true;
		//if (IsKeyJustDown(input, r))
		//	temp = !temp;
		if (temp)
			rs->irradianceMap.Bind(rs, ShaderStage::PIXEL, 5);
		else
			rs->environmentMap.Bind(rs, ShaderStage::PIXEL, 5);

		rs->meshes[(int32)ModelId::Value::CUBE].Bind(rs);
		rs->meshes[(int32)ModelId::Value::CUBE].DrawIndexed(rs);

		SetDepthState(rs, rs->depthLessEqualState);
		SetRasterState(rs, rs->rasterNormal);

		for (int32 i = 0; i < renderGroup->opaqueEntityCount; i++)
		{
			OpaqueRenderEntry* entry = &renderGroup->opaqueRenderEntries[i];

			Mat4f m = entry->transform.CalculateTransformMatrix();
			ShaderConstBuffer* vC0 = &rs->vConstBuffers[0];
			vC0->CopyMat4fIntoShaderBuffer(m * v * p, false);
			vC0->CopyMat4fIntoShaderBuffer(m, false);
			vC0->CopyMat4fIntoShaderBuffer(Inverse(m), false);
			vC0->CommitChanges();

			Assert(entry->renderComp.modelId.IsValid(), "Invalid model id");

			rs->shaders[(int32)ShaderId::Value::BASIC_PBR].Bind();
#if 0
			rs->textures[(int32)TextureId::BOOMBOX_BASECOLOR].Bind(rs, ShaderStage::PIXEL, 0);
			rs->textures[(int32)TextureId::BOOMBOX_OCCLUSIONROUGHNESSMETALLIC].Bind(rs, ShaderStage::PIXEL, 1);
			rs->textures[(int32)TextureId::BOOMBOX_NORMAL].Bind(rs, ShaderStage::PIXEL, 2);
			rs->textures[(int32)TextureId::BOOMBOX_EMISSIVE].Bind(rs, ShaderStage::PIXEL, 3);
#else
			if (entry->renderComp.material.albedoTex != TextureId::Value::INVALID)
				rs->textures[(int32)entry->renderComp.material.albedoTex].Bind(rs, ShaderStage::PIXEL, 0);
#endif
			rs->meshes[entry->renderComp.modelId].Bind(rs);
			rs->meshes[entry->renderComp.modelId].DrawIndexed(rs);
		}

		SetDepthState(rs, rs->depthNormal);

		DEBUGRenderAndFlushDebugDraws(rs);
	}

	void PresentFrame(RenderState* rs, bool32 vsync)
	{
		DXCHECK(rs->swapChain->Present(vsync, 0));
	}


	void TextureArrayInstance::Bind(RenderState* rs, ShaderStage shaderStage, int32 register_)
	{
		Assert(register_ >= 0, "Shader register invalid");

		this->currentRegister = register_;
		this->shaderStage = shaderStage;

		switch (shaderStage)
		{
		case ShaderStage::VERTEX: break;
		case ShaderStage::PIXEL:
		{
			DXINFO(rs->context->PSSetShaderResources(register_, 1, &shaderView));
		}break;
		case ShaderStage::COMPUTE:
		{
			DXINFO(rs->context->CSSetShaderResources(register_, 1, &shaderView));
		}break;
		}
	}

	void TextureArrayInstance::Unbind(RenderState* rs)
	{
		ID3D11ShaderResourceView* nullTexture = nullptr;
		switch (shaderStage)
		{
		case ShaderStage::VERTEX: break;
		case ShaderStage::PIXEL:
		{
			DXINFO(rs->context->PSSetShaderResources(currentRegister, 1, &nullTexture));
		}break;
		case ShaderStage::COMPUTE:
		{
			DXINFO(rs->context->CSSetShaderResources(currentRegister, 1, &nullTexture));
		}break;
		}
	}

}