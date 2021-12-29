#include "core/SolarRenderer.h"
#include "game/components/SolarCamera.h"
#include "DX11Renderer.h"
#include "game/TankGame.h"



namespace cm
{
	static void InitializeDirectXDebugDrawing()
	{
		GetRenderState();
		GetDebugState();

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

		DXCHECK(rs->device->CreateBuffer(&vertex_desc, &vertex_res, &rs->debug.vertex_buffer));

		GetAssetState();
		auto shaders = as->shaders.GetValueSet();

		ShaderAsset asset = GetAssetFromName(shaders, "debug_line");
		rs->debug.shader = ShaderInstance::CreateGraphics(asset);
	}

	void DEBUGRenderAndFlushDebugDraws()
	{
		GetRenderState();
		GetDebugState();

		RenderDebug* rd = &rs->debug;

		D3D11_MAPPED_SUBRESOURCE resource = {};
		DXCHECK(rs->context->Map(rd->vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));

		memcpy(resource.pData, ds->vertex_data, ds->vertex_size_bytes);
		DXINFO(rs->context->Unmap(rd->vertex_buffer, 0));

		uint32 offset = 0;
		uint32 vertex_stride_bytes = ds->vertex_stride * sizeof(real32);
		DXINFO(rs->context->IASetVertexBuffers(0, 1, &rd->vertex_buffer, &vertex_stride_bytes, &offset));

		//////////////////////////////////
		//////////////////////////////////

		RenderCommand::BindShader(rs->debug.shader);

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

	static void CreateDeviceAndSwapChain()
	{
		GetRenderState();

		LOGTODO("D3D11 is always using debug context");
		D3D_FEATURE_LEVEL feature_level;

		PlatformState* ws = PlatformState::Get();

		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.RefreshRate.Denominator = 1000;
		sd.BufferDesc.RefreshRate.Numerator = 60000;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 2; // @NOTE This implies just the back buffer, ie we have two buffers
		sd.OutputWindow = (HWND)ws->window;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		sd.Flags = 0;

		DXCHECK(D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			D3D11_CREATE_DEVICE_DEBUG,
			NULL, 0, // @NOTE: Pick the highest feature level
			D3D11_SDK_VERSION,
			&sd,
			&rs->swapChain.swapChain,
			&rs->device,
			&feature_level,
			&rs->context));

		ID3D11RenderTargetView* render_target = nullptr;
		ID3D11DepthStencilView* depth_target = nullptr;
		ID3D11ShaderResourceView* shaderView = nullptr;
		// @NOTE: Get back buffer
		{
			ID3D11Resource* back_buffer = nullptr;
			DXCHECK(rs->swapChain.swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&back_buffer));
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

		rs->swapChain.depthView = depth_target;
		rs->swapChain.depthShaderView = shaderView;
		rs->swapChain.renderView = render_target;
	}


	inline Vec4f RGBToSRGB(int32 r, int32 g, int32 b, int32 a)
	{
		return Vec4f((real32)r, (real32)g, (real32)b, (real32)a) / Vec4f(255.0f);
	}

	static void CreateAllRasterState()
	{
		GetRenderState();

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

			DXCHECK(rs->device->CreateRasterizerState(&rs_desc, &rs->rasterBackFaceCullingState));
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

	static void CreateAllDepthStencilState()
	{
		GetRenderState();

		{
			D3D11_DEPTH_STENCIL_DESC ds = {};
			ds.DepthEnable = TRUE;
			ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			ds.DepthFunc = D3D11_COMPARISON_LESS;
			ds.StencilEnable = FALSE;

			DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->depthLessState));
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

	static void CreateAllBlendState()
	{
		GetRenderState();

		{
			D3D11_BLEND_DESC blend_desc = {};
			blend_desc.RenderTarget[0].BlendEnable = true;
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

	static void CreateAllSamplerState()
	{
		GetRenderState();

		rs->pointRepeat = SamplerInstance::Create(TextureFilterMode::POINT, TextureWrapMode::REPEAT);
		rs->bilinearRepeat = SamplerInstance::Create(TextureFilterMode::BILINEAR, TextureWrapMode::CLAMP_EDGE);
		rs->trilinearRepeat = SamplerInstance::Create(TextureFilterMode::TRILINEAR, TextureWrapMode::REPEAT);
		rs->shadowPFC = SamplerInstance::CreateShadowPFC();

		RenderCommand::BindSampler(rs->pointRepeat, 0);
		RenderCommand::BindSampler(rs->bilinearRepeat, 1);
		RenderCommand::BindSampler(rs->trilinearRepeat, 2);
		RenderCommand::BindSampler(rs->shadowPFC, 3);
	}

	static void CreateAllStaticMeshes()
	{
		GetRenderState();
		GetAssetState();

		ManagedArray<ModelAsset> models = as->models.GetValueSet();
		for (uint32 assetIndex = 0; assetIndex < models.GetCount(); assetIndex++)
		{
			StaticMesh mesh = StaticMesh::Create(models[assetIndex]);
			rs->meshes.Put(mesh.id, mesh);
		}

		rs->quad = StaticMesh::CreateScreenSpaceQuad();
		rs->meshes.Put(rs->quad.id, rs->quad);

		ModelAsset cube = GetAssetFromName(models, "cube");
		cube.id = 2;
		rs->cube = StaticMesh::Create(cube);
		rs->meshes.Put(rs->cube.id, rs->cube);
	}

	static void CreateAllTextures()
	{
		GetRenderState();
		GetAssetState();

		ManagedArray<TextureAsset> textures = as->textures.GetValueSet();
		for (uint32 assetIndex = 0; assetIndex < textures.GetCount(); assetIndex++)
		{
			TextureInstance texture = TextureInstance::Create(textures[assetIndex]);
			rs->textures.Put(texture.id, texture);
		}
	}

	static void LoadFont(const FontAsset& font)
	{
		GetRenderState();

		for (uint32 i = 0; i < font.chars.count; i++)
		{
			const FontCharacter& c = font.chars[i];
			if (c.size.x > 0 && c.size.y > 0)
			{
				rs->fontTextures.Add(TextureInstance::Create(c));
			}
			else
			{
				rs->fontTextures.Add({});
			}
		}

		rs->fontMesh = StaticMesh::Create(nullptr, sizeof(float) * 6 * 4);
	}

	static void RenderText(const CString& text, real32 x, real32 y, real32 scale, Vec3f color)
	{
		GetRenderState();
		GetAssetState();
		GetPlatofrmState();
		RenderCommand::BindShader(rs->textShader);
		RenderCommand::SetRasterState(rs->rasterNoFaceCullState);
		for (int32 i = 0; i < text.GetLength(); i++)
		{
			FontCharacter ch = {};
			TextureInstance te = {};
			for (uint32 j = 0; j < rs->fontTextures.count; j++)
			{
				if (text[i] == as->font.chars[j].character)
				{
					ch = as->font.chars[j];
					te = rs->fontTextures[j];
					break;
				}
			}

			if (ch.size.x > 0)
			{
				real32 xpos = x + ch.bearing.x * scale;
				real32 ypos = y + (ch.size.y - ch.bearing.y) * scale;

				real32 w = ch.size.x * scale;
				real32 h = ch.size.y * scale;

				real32 vertices[] = {
					 xpos,     ypos - h,   0.0f, 0.0f,
					 xpos,     ypos,       0.0f, 1.0f,
					 xpos + w, ypos,       1.0f, 1.0f,

					 xpos,     ypos - h,   0.0f, 0.0f,
					 xpos + w, ypos,       1.0f, 1.0f,
					 xpos + w, ypos - h,   1.0f, 0.0f
				};

				rs->fontMesh.UpdateVertexBuffer(vertices, sizeof(vertices));

				uint32 offset = 0;
				RenderCommand::BindTexture(te, 0);
				DXINFO(rs->context->IASetVertexBuffers(0, 1, &rs->fontMesh.vertexBuffer, &rs->fontMesh.strideBytes, &offset));
				DXINFO(rs->context->Draw(6, 0));

			}
			x += (ch.advance >> 6) * scale;
		}

		RenderCommand::SetRasterState(rs->rasterBackFaceCullingState);
	}

	static void RenderRect(real32 x, real32 y, real32 w, real32 h, AssetId textureId, const Vec4f& colour)
	{
		GetRenderState();

		RenderCommand::BindShader(rs->quadShader);
		RenderCommand::SetRasterState(rs->rasterNoFaceCullState);
		RenderCommand::SetDepthState(rs->depthOffState);

		rs->uiConstBuffer.data.colour = colour;
		rs->uiConstBuffer.data.sizePos = Vec4f(w, h, x, y);

		if (textureId == INVALID_ASSET_ID)
		{
			rs->uiConstBuffer.data.uiUses = Vec4i(0, 0, 0, 0);
		}
		else
		{
			TextureInstance* texture = rs->textures.Get(textureId);
			RenderCommand::BindTexture(*texture, 7);
			rs->uiConstBuffer.data.uiUses = Vec4i(7, 0, 0, 0);
		}

		RenderCommand::UpdateConstBuffer(rs->uiConstBuffer);
		RenderCommand::BindAndDrawMesh(rs->quad);

		RenderCommand::SetDepthState(rs->depthLessState);
		RenderCommand::SetRasterState(rs->rasterBackFaceCullingState);
	}

	bool32 Renderer::Initialize()
	{
		RenderState::Initialize(GameMemory::PushPermanentStruct<RenderState>());

		GetRenderState();
		GetAssetState();

		InitializeDirectXDebugLogging();
		CreateDeviceAndSwapChain();
		InitializeDirectXDebugDrawing();
		CreateAllRasterState();
		CreateAllDepthStencilState();
		CreateAllBlendState();
		CreateAllSamplerState();

		LoadFont(as->font);

		rs->modelConstBuffer = ShaderConstBuffer<ShaderConstBufferModel>::Create();
		RenderCommand::BindShaderConstBuffer(rs->modelConstBuffer, ShaderStage::VERTEX, 0);
		rs->viewConstBuffer = ShaderConstBuffer<ShaderConstBufferView>::Create();
		RenderCommand::BindShaderConstBuffer(rs->viewConstBuffer, ShaderStage::VERTEX, 1);
		rs->lightingConstBuffer = ShaderConstBuffer<ShaderConstBufferLightingInfo>::Create();
		RenderCommand::BindShaderConstBuffer(rs->lightingConstBuffer, ShaderStage::PIXEL, 0);
		rs->uiConstBuffer = ShaderConstBuffer<ShaderConstBufferUIData>::Create();
		RenderCommand::BindShaderConstBuffer(rs->uiConstBuffer, ShaderStage::PIXEL, 4);

		ManagedArray<ShaderAsset> shaders = as->shaders.GetValueSet();
		rs->unlitShader = ShaderInstance::CreateGraphics(GetAssetFromName(shaders, "unlit"));
		rs->phongShader = ShaderInstance::CreateGraphics(GetAssetFromName(shaders, "phong"));
		rs->quadShader = ShaderInstance::CreateGraphics(GetAssetFromName(shaders, "ui_quad"));
		rs->skyboxShader = ShaderInstance::CreateGraphics(GetAssetFromName(shaders, "skybox"));
		rs->eqiToCubeShader = ShaderInstance::CreateGraphics(GetAssetFromName(shaders, "equirectangular_to_cubemap"));

		ShaderAsset textShader = GetAssetFromName(shaders, "text");
		textShader.vertexLayout = VertexShaderLayout::TEXT;
		rs->textShader = ShaderInstance::CreateGraphics(textShader);


		CreateAllStaticMeshes();
		CreateAllTextures();

		ManagedArray<TextureAsset> textures = as->textures.GetValueSet();
		TextureInstance* eqi = rs->textures.Get(GetAssetFromName(textures, "FS002_Day_Sunless").id);
		rs->skyboxMap = ConvertEqiTextureToCubeMap(512, *eqi);

		//rs->testBuffer = CreateShaderBuffer(rs, sizeof(Mat4f) * 3);
		//rs->testBuffer.BindShaderBuffer(ShaderStage::VERTEX, 0);

		return true;
	}

	void Renderer::RenderGame(EntityRenderGroup* renderGroup)
	{
		GetRenderState();
		GetPlatofrmState();

		Mat4f p = renderGroup->playerCamera.GetProjectionMatrix();
		Mat4f v = renderGroup->playerCamera.GetViewMatrix();

		rs->viewConstBuffer.data.persp = p;
		rs->viewConstBuffer.data.view = v;
		rs->viewConstBuffer.data.screeenProjection = OrthographicLH(0.0f, (real32)ps->clientWidth, 0.0f, (real32)ps->clientHeight, 0.0f, 1.0f);
		RenderCommand::UpdateConstBuffer(rs->viewConstBuffer);

		RenderCommand::ClearRenderTarget(rs->swapChain.renderView, Vec4f(0.2f, 0.2f, 0.2f, 1.0f));
		RenderCommand::ClearDepthBuffer(rs->swapChain.depthView);
		RenderCommand::BindRenderTargets(rs->swapChain.renderView, rs->swapChain.depthView);
		RenderCommand::SetTopology(Topology::Value::TRIANGLE_LIST);
		RenderCommand::SetViewportState((real32)ps->clientWidth, (real32)ps->clientHeight);

		RenderCommand::SetDepthState(rs->depthOffState);
		RenderCommand::SetRasterState(rs->rasterNoFaceCullState);

		RenderCommand::BindShader(rs->skyboxShader);
		RenderCommand::BindCubeMap(rs->skyboxMap, 5);
		RenderCommand::BindAndDrawMesh(rs->cube);

		RenderCommand::SetDepthState(rs->depthLessState);
		RenderCommand::SetRasterState(rs->rasterBackFaceCullingState);

		RenderCommand::BindShader(rs->phongShader);

		rs->lightingConstBuffer.data.viewPos = Vec4f(renderGroup->playerCamera.transform.position, 0.0f);
		RenderCommand::UpdateConstBuffer(rs->lightingConstBuffer);

		AssetId lastModelId = 0;
		AssetId lastTextureId = 0;
		for (uint32 entryIndex = 0; entryIndex < renderGroup->entries.count; entryIndex++)
		{
			RenderEntry* entry = &renderGroup->entries[entryIndex];

			Mat4f m = entry->transform.CalculateTransformMatrix();
			Mat4f mvp = m * v * p;

			rs->modelConstBuffer.data.mvp = mvp;
			rs->modelConstBuffer.data.model = m;
			rs->modelConstBuffer.data.invM = m;

			// @TODO: Check for null asset id first!!

			TextureInstance* texture = rs->textures.Get(entry->textureId);
			if (texture)
			{
				RenderCommand::BindTexture(*texture, 0);
			}
			else
			{
				//Debug::LogInfo("Could not find texture");
			}

			StaticMesh* mesh = rs->meshes.Get(entry->modelId);
			if (mesh)
			{
				lastModelId = entry->modelId;
				RenderCommand::UpdateConstBuffer(rs->modelConstBuffer);
				RenderCommand::BindAndDrawMesh(*mesh);
			}
			else
			{
				//Debug::LogInfo("Could not find mesh");
			}
		}

		for (uint32 i = 0; i < renderGroup->uiState.uiElements.count; i++)
		{
			UIElement* el = &renderGroup->uiState.uiElements[i];

			switch (el->type)
			{
			case UIElementType::TEXT:
			{
				UIText* text = &el->text;

				GetAssetState();

				real32 w = as->font.GetWidthOfText(text->text, text->scale);
				real32 x = text->oX * ps->clientWidth - w / 2.0f;
				real32 y = text->oY * ps->clientHeight;

				RenderText(text->text, x, y, text->scale, Vec3f(1));
			}break;
			case UIElementType::RECT:
			{
				UIRect* rect = &el->rect;

				real32 x = rect->oX * ps->clientWidth;
				real32 y = rect->oY * ps->clientHeight;
				real32 w = rect->width * ps->clientWidth;
				real32 h = rect->height * ps->clientHeight;

				RenderRect(x, y, w, h, rect->texture, rect->colour);
			}break;

			}
		}

		DEBUGRenderAndFlushDebugDraws();
	}

	void Renderer::PresentFrame()
	{
		GetRenderState();
		DXCHECK(rs->swapChain.swapChain->Present(1, 0));
	}

	void Renderer::Shutdown()
	{
	}
}

