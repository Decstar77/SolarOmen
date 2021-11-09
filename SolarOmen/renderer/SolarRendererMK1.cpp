#include "SolarRendererMK1.h"
#if 0
#include "../Debug.h"

#include <stack>
namespace cm
{
	static inline DXGI_FORMAT TextureFormatToD3D(const TextureFormat& format)
	{
		switch (format)
		{
		case TextureFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
		case TextureFormat::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
		case TextureFormat::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
		case TextureFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TextureFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		default: Assert(0, "TextureFormatToD3D ??");
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	static inline D3D11_TEXTURE_ADDRESS_MODE TextureWrapModeToD3D(const TextureWrapMode& wrap)
	{
		switch (wrap)
		{
		case TextureWrapMode::REPEAT: return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureWrapMode::CLAMP_EDGE:return D3D11_TEXTURE_ADDRESS_CLAMP;
		default: Assert(0, "TextureWrapModeToD3D ??");
		}

		return D3D11_TEXTURE_ADDRESS_WRAP;
	}

	static inline D3D11_FILTER TextureFilterModeToD3D(const TextureFilterMode& mode)
	{
		switch (mode)
		{
		case TextureFilterMode::POINT:		return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case TextureFilterMode::BILINEAR:	return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TextureFilterMode::TRILINEAR:	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		default: Assert(0, "TextureFilterModeToD3D ??");
		}

		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	static inline int32 TextureUsageToD3DBindFlags(const TextureUsage& usage)
	{
		switch (usage)
		{
		case TextureUsage::NONE:  return 0;
		case TextureUsage::SHADER_RESOURCE: return D3D11_BIND_SHADER_RESOURCE;
		case TextureUsage::RENDER_TARGET: return D3D11_BIND_RENDER_TARGET;
		case TextureUsage::COMPUTER_SHADER_RESOURCE: return D3D11_BIND_UNORDERED_ACCESS;
		default: Assert(0, "TextureUsageToD3DBindFlags ??");
		}

		return 0;
	}

	static void SortPrimitives(BVH* bvh, BVHNode* node, AABB* boxes, Vec3f* centers)
	{
		int32 axis = GetAABBLargestAxis(node->box);
		int32 startIndex = node->firstIndex;
		int32 endIndex = node->firstIndex + node->primCount;

		std::sort(bvh->primitiveIndices + startIndex, bvh->primitiveIndices + endIndex,
			[&](int32 i, int32 j) {
				Vec3f c1 = centers[i];
				Vec3f c2 = centers[j];
				return c1[axis] < c2[axis];
			});
#if 0
		LOG("Axis " << axis);
		for (int32 i = startIndex; i < endIndex; i++)
		{
			int32 primitiveIndex = bvh->primitiveIndices[node->firstIndex + i];
			AABB primitiveBox = boxes[primitiveIndex];
			Vec3f c1 = GetAABBCenter(primitiveBox);
			LOG(c1[axis]);
		}
#endif
	}

	// @TODO: Move this !!
	static void BuildBVHRecursive(BVH* bvh, int32 nodeIndex, int32* nodeCount, AABB* boxes, Vec3f* centers)
	{

		BVHNode* node = &bvh->nodes[nodeIndex];
		Assert(node->IsLeaf(), "BVH node is not leaf !!");

		node->box = CreateAABBEmpty();

		for (int32 i = 0; i < node->primCount; i++)
		{
			int32 primitiveIndex = bvh->primitiveIndices[node->firstIndex + i];
			AABB primitiveBox = boxes[primitiveIndex];
			node->box = ExtendAABB(node->box, primitiveBox);
		}

		int32 maxPrimCount = bvh->maxPrimCount;
		if (node->primCount <= maxPrimCount)
			return;

		SortPrimitives(bvh, node, boxes, centers);

		int32 firstChild = *nodeCount;
		int32 firstRight = node->firstIndex + node->primCount / 2;

		BVHNode* left = &bvh->nodes[firstChild];
		BVHNode* right = &bvh->nodes[firstChild + 1];
		*nodeCount += 2;

		left->firstIndex = node->firstIndex;
		right->firstIndex = firstRight;

		left->primCount = firstRight - node->firstIndex;
		right->primCount = node->primCount - left->primCount;

		node->firstIndex = firstChild;
		node->primCount = 0;

		BuildBVHRecursive(bvh, firstChild, nodeCount, boxes, centers);
		BuildBVHRecursive(bvh, firstChild + 1, nodeCount, boxes, centers);


	}

	int32 FindClosestNode(const std::vector<BVHNode>& nodes, int32 index) {
		static int32 search_radius = 14;

		int32 begin = index > search_radius ? index - search_radius : 0;
		int32 end = index + search_radius + 1 < (int32)nodes.size() ? index + search_radius + 1 : (int32)nodes.size();

		const BVHNode* firstNode = &nodes[index];

		int32 bestIndex = 0;
		real32 bestDistance = REAL_MAX;
		for (int32 i = begin; i < end; ++i) {
			if (i == index)
			{
				continue;
			}

			const BVHNode* second_node = &nodes[i];
			AABB box = ExtendAABB(firstNode->box, second_node->box);

			real32 distance = GetAABBHalfArea(box);
			if (distance < bestDistance) {
				bestDistance = distance;
				bestIndex = i;
			}
		}

		return bestIndex;
	}

	// @TODO: Move this !!
	void BuildBVH(BVH* bvh, AABB* boxes, Vec3f* centers, int32 count)
	{
		Assert(count < ArrayCount(bvh->primitiveIndices), "BVH Array count thing");
#if 1
		bvh->primitiveCount = count;

		for (int32 i = 0; i < count; i++)
		{
			bvh->primitiveIndices[i] = i;
		}

		int32 maxSize = 2 * count - 1;
		Assert(maxSize < ArrayCount(bvh->nodes), "BVH Array count thing");

		bvh->nodes[0].primCount = count;
		bvh->nodes[0].firstIndex = 0;

		int32 nodeCount = 1;
		BuildBVHRecursive(bvh, 0, &nodeCount, boxes, centers);
		bvh->nodeCount = nodeCount;

#else
		AABB globalBox = CreateAABBEmpty();

		for (int32 i = 0; i < count; i++)
		{
			globalBox.min = Min(globalBox.min, centers[i]);
			globalBox.max = Max(globalBox.max, centers[i]);
		}


		// @TODO: EVIL STD VECTOR!!
		std::vector<uint64> mortons(count);
		for (int32 i = 0; i < count; i++)
		{
			Vec3f t1 = Max(Vec3f(0), (centers[i] - globalBox.min) * (Vec3f(Morton::GridDim) / GetAABBDiagonal(globalBox)));
			Vec3f gridPos = Min(Vec3f(Morton::GridDim - 1), t1);

			mortons[i] = Morton::Encode((uint64)gridPos[0], (uint64)gridPos[1], (uint64)gridPos[2]);
		}

		for (int32 i = 0; i < count; i++)
		{
			bvh->primitiveIndices[i] = i;
		}

		std::sort(bvh->primitiveIndices, bvh->primitiveIndices + count,
			[&](uint64 i, uint64 j) {
				return mortons[i] < mortons[j];
			});

		std::vector<BVHNode> currentNodes(count);
		for (int32 i = 0; i < count; i++) {

			currentNodes[i].primCount = 1;
			currentNodes[i].firstIndex = i;
			currentNodes[i].box = boxes[bvh->primitiveIndices[i]];
		}

		// Merge nodes until there is only one left
		int32 maxSize = 2 * count - 1;
		Assert(maxSize < ArrayCount(bvh->nodes), "BVH Array count thing");

		std::vector<BVHNode> nextNodes;
		std::vector<int32> mergeIndex(count);

		int32 insertionIndex = maxSize;

		while (currentNodes.size() > 1) {
			for (int32 i = 0; i < currentNodes.size(); ++i)
				mergeIndex[i] = FindClosestNode(currentNodes, i);

			nextNodes.clear();
			for (size_t i = 0; i < currentNodes.size(); ++i) {
				int32 j = mergeIndex[i];

				// @NOTE: The two nodes should be merged if they agree on their respective merge indices.
				if (i == mergeIndex[j])
				{
					// @NOTE: Since we only need to merge once, we only merge if the first index is less than the second.
					if (i > j)
						continue;

					// @NOTE: Reserve space in the target array for the two children
					Assert(insertionIndex >= 2, "No space ?");

					insertionIndex -= 2;

					bvh->nodes[insertionIndex + 0] = currentNodes[i];
					bvh->nodes[insertionIndex + 1] = currentNodes[j];

					// @NOTE: Create the parent node and place it in the array for the next iteration
					BVHNode parent = {};

					parent.box = ExtendAABB(currentNodes[i].box, currentNodes[j].box);
					parent.firstIndex = insertionIndex;
					parent.primCount = 0;
					nextNodes.push_back(parent);
				}
				else
				{
					// The current node should be kept for the next iteration
					nextNodes.push_back(currentNodes[i]);
				}
			}

			std::swap(nextNodes, currentNodes);
		}

		Assert(insertionIndex == 1, "huh?");

		bvh->nodeCount = count;
		bvh->nodes[0] = currentNodes[0];
#endif
	}

	void DEBUGDrawBVH(RenderState* rs, BVH* bvh, bool32 leafOnly, int32 count)
	{
		if (count < 0)
		{
			for (int32 i = 0; i < bvh->nodeCount; i++)
			{
				BVHNode* node = &bvh->nodes[i];
				if (leafOnly)
				{
					if (node->IsLeaf())
					{
						DEBUGDrawAABB(node->box);
					}
				}
				else
				{
					DEBUGDrawAABB(node->box);
				}
			}
		}
		else
		{
			for (int32 i = 0; i < bvh->nodeCount; i++)
			{
				BVHNode* node = &bvh->nodes[i];
				if (leafOnly)
				{
					if (node->IsLeaf() && i == count)
					{
						DEBUGDrawAABB(node->box);
					}
				}
				else
				{
					if (i == count)
					{
						DEBUGDrawAABB(node->box);
					}
				}
			}
		}
	}

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



	ShaderInstance DEBUGCreateShaderFromSource(RenderState* rs, CString vertex_file, CString pixel_file)
	{
		PlatformFile vertex_code = DEBUGLoadEntireFile(vertex_file, false);
		PlatformFile pixel_code = DEBUGLoadEntireFile(pixel_file, false);

		ID3DBlob* vs_shader = nullptr;
		ID3DBlob* vs_error = nullptr;

		DXCHECK(D3DCompile(
			vertex_code.data,
			vertex_code.size_bytes,
			NULL, NULL, NULL,
			"main", "vs_5_0",
			D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_DEBUG,
			NULL, &vs_shader, &vs_error
		));


		ID3DBlob* ps_shader = nullptr;
		ID3DBlob* ps_error = nullptr;

		DXCHECK(D3DCompile(
			pixel_code.data,
			pixel_code.size_bytes,
			NULL, NULL, NULL,
			"main", "ps_5_0",
			D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_DEBUG,
			NULL, &ps_shader, &ps_error
		));

		if (vs_error)
		{
			LOG((char*)vs_error->GetBufferPointer());
		}

		if (ps_error)
		{
			LOG((char*)ps_error->GetBufferPointer());
		}

		ShaderInstance shader = {};
		if (vs_shader && ps_shader)
		{
			D3D11_INPUT_ELEMENT_DESC pos_desc = {};
			pos_desc.SemanticName = "Position";
			pos_desc.SemanticIndex = 0;
			pos_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pos_desc.InputSlot = 0;
			pos_desc.AlignedByteOffset = 0;
			pos_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pos_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC nrm_desc = {};
			nrm_desc.SemanticName = "Normal";
			nrm_desc.SemanticIndex = 0;
			nrm_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			nrm_desc.InputSlot = 0;
			nrm_desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			nrm_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			nrm_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC txc_desc = {};
			txc_desc.SemanticName = "TexCord";
			txc_desc.SemanticIndex = 0;
			txc_desc.Format = DXGI_FORMAT_R32G32_FLOAT;
			txc_desc.InputSlot = 0;
			txc_desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			txc_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			txc_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC layouts[] = { pos_desc, nrm_desc, txc_desc };

			DXCHECK(rs->device->CreateInputLayout(layouts, 3, vs_shader->GetBufferPointer(),
				vs_shader->GetBufferSize(), &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(vs_shader->GetBufferPointer(),
				vs_shader->GetBufferSize(), NULL, &shader.vs_shader));

			DXCHECK(rs->device->CreatePixelShader(ps_shader->GetBufferPointer(),
				ps_shader->GetBufferSize(), NULL, &shader.ps_shader));
		}

		DXRELEASE(ps_shader);
		DXRELEASE(vs_shader);

		DXRELEASE(ps_error);
		DXRELEASE(vs_error);

		DEBUGFreeFile(&vertex_code);
		DEBUGFreeFile(&pixel_code);

		return shader;
	}

	ComputeShaderInstance DEBUGCreateComputeShaderFromBinary(RenderState* rs, CString file)
	{
		PlatformFile code = DEBUGLoadEntireFile(file, false);

		Assert(code.data, "Could not open file");

		ComputeShaderInstance result = {};
		DXCHECK(rs->device->CreateComputeShader(code.data, code.size_bytes, nullptr, &result.cs_shader));

		DEBUGFreeFile(&code);

		return result;
	}


	void InitializeDirectXDebugLogging(RenderState* rs)
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

	void InitializeDirectXDebugDrawing(RenderState* rs, AssetState* as)
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

		// @HACK: Should debug stuff even go into shader table ?
		int32 shaderIndex = DEBUGCreateShaderFromBinary(rs, as, VertexShaderLayout::P,
			"../Assets/Processed/Shaders/debug_line.vert.cso", "../Assets/Processed/Shaders/debug_line.pixl.cso");

		rd->shader = rs->shaders[shaderIndex];
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

		BindShader(rs, &rd->shader);

		//////////////////////////////////
		//////////////////////////////////

		DXINFO(rs->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST));

		DXINFO(rs->context->Draw(ds->next_vertex_index, 0));
		//DXINFO(rs->context->Draw(rd->vertex_count, 0));

		DXINFO(rs->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		//////////////////////////////////
		//////////////////////////////////

		ZeroMemory(ds->vertex_data, ds->vertex_size_bytes);
		ds->next_vertex_index = 0;
	}

	void InitializeDirectX(HWND window, RenderState* rs, AssetState* as)
	{
		LOGTODO("D3D11 is always using debug context");
		InitializeDirectXDebugLogging(rs);

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
		sd.OutputWindow = window;
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
			&rs->swapchain.swapchain,
			&rs->device,
			&feature_level,
			&rs->context));

		InitializeDirectXDebugDrawing(rs, as);

		ID3D11RenderTargetView* render_target = nullptr;
		ID3D11DepthStencilView* depth_target = nullptr;

		ID3D11Resource* back_buffer = nullptr;
		DXCHECK(rs->swapchain.swapchain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&back_buffer));
		DXCHECK(rs->device->CreateRenderTargetView(back_buffer, nullptr, &render_target));
		back_buffer->Release();

		D3D11_DEPTH_STENCIL_DESC ds = {};
		ds.DepthEnable = TRUE;
		ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		ds.DepthFunc = D3D11_COMPARISON_LESS;
		ds.StencilEnable = FALSE;

		DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->ds_state));

		DXINFO(rs->context->OMSetDepthStencilState(rs->ds_state, 1));

		RECT window_rect;
		GetClientRect(window, &window_rect);

		ID3D11Texture2D* depth_texture = nullptr;
		D3D11_TEXTURE2D_DESC depth_ds = {};
		depth_ds.Width = (uint32)(window_rect.right - window_rect.left);
		depth_ds.Height = (uint32)(window_rect.bottom - window_rect.top);
		depth_ds.MipLevels = 1;
		depth_ds.ArraySize = 1;
		depth_ds.Format = DXGI_FORMAT_D32_FLOAT;
		depth_ds.SampleDesc.Count = 1;
		depth_ds.SampleDesc.Quality = 0;
		depth_ds.Usage = D3D11_USAGE_DEFAULT;
		depth_ds.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		DXINFO(rs->device->CreateTexture2D(&depth_ds, nullptr, &depth_texture));

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
		depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
		depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_view_dsc.Texture2D.MipSlice = 0;
		DXCHECK(rs->device->CreateDepthStencilView(depth_texture, &depth_view_dsc, &depth_target));

		rs->swapchain.render_target.depth_target = depth_target;
		rs->swapchain.render_target.depth_texture = depth_texture;
		rs->swapchain.render_target.renderTarget0 = render_target;
		rs->swapchain.render_target.colourTexture0.texture = nullptr;

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

		DXCHECK(rs->device->CreateRasterizerState(&rs_desc, &rs->rs_standard_state));

		DXINFO(rs->context->RSSetState(rs->rs_standard_state));

		rs_desc = {};
		rs_desc.FillMode = D3D11_FILL_SOLID;
		rs_desc.CullMode = D3D11_CULL_FRONT;
		rs_desc.FrontCounterClockwise = FALSE;
		rs_desc.DepthBias = 0;
		rs_desc.DepthBiasClamp = 1.0f;
		rs_desc.SlopeScaledDepthBias = 0.0f;
		rs_desc.DepthClipEnable = TRUE;
		rs_desc.MultisampleEnable = FALSE;
		rs_desc.AntialiasedLineEnable = FALSE;

		DXCHECK(rs->device->CreateRasterizerState(&rs_desc, &rs->frontFaceCullingState));

		rs_desc.FillMode = D3D11_FILL_SOLID;
		rs_desc.CullMode = D3D11_CULL_NONE;
		rs_desc.FrontCounterClockwise = FALSE;
		rs_desc.DepthBias = 0;
		rs_desc.DepthBiasClamp = 1.0f;
		rs_desc.SlopeScaledDepthBias = 0.0f;
		rs_desc.DepthClipEnable = TRUE;
		rs_desc.MultisampleEnable = FALSE;
		rs_desc.AntialiasedLineEnable = FALSE;

		DXCHECK(rs->device->CreateRasterizerState(&rs_desc, &rs->noFaceCullState));

		ds.DepthEnable = FALSE;
		ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		ds.DepthFunc = D3D11_COMPARISON_ALWAYS;
		ds.StencilEnable = FALSE;
		DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->depthOffState));

		ds.DepthEnable = TRUE;
		ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		ds.StencilEnable = FALSE;
		DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->depthLessEqualState));

		ds.DepthEnable = TRUE;
		ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		ds.DepthFunc = D3D11_COMPARISON_LESS;
		ds.StencilEnable = TRUE;
		ds.StencilReadMask = 0xFF;
		ds.StencilWriteMask = 0xFF;

		ds.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		ds.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		ds.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		ds.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		ds.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		ds.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		ds.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		ds.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		DXCHECK(rs->device->CreateDepthStencilState(&ds, &rs->shadow_depth_state));

		real32 quad_data[] = {
			-1, 1, 0,	0, 0, -1,	0, 0,
			1, -1, 0,	0, 0, -1,	1, 1,
			-1, -1, 0,	0, 0, -1,	0, 1,
			1, 1, 0,	0, 0, -1,	1, 0
		};

		uint32 index_data[] = {
			0, 1, 2, 0, 3, 1
		};

		rs->screen_space_quad = CreateMeshInstance(rs, quad_data, ArrayCount(quad_data), index_data, ArrayCount(index_data));

		D3D11_BLEND_DESC blend_desc = {};
		blend_desc.RenderTarget[0].BlendEnable = FALSE;
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		DXCHECK(rs->device->CreateBlendState(&blend_desc, &rs->blend_state));

		rs->context->OMSetBlendState(rs->blend_state, nullptr, 0xffffffff);
	}

	RenderTargetCubeMap CreatePointLightCubeMap(RenderState* rs)
	{
		RenderTargetCubeMap result = {};

		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = 1024;
			desc.Height = 1024;
			desc.MipLevels = 1;
			desc.ArraySize = 6;
			desc.Format = TextureFormatToD3D(TextureFormat::R32G32_FLOAT);
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			DXINFO(rs->device->CreateTexture2D(&desc, nullptr, &result.cubeMap));
		}

		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = 1024;
			desc.Height = 1024;
			desc.MipLevels = 1;
			desc.ArraySize = 6;
			desc.Format = DXGI_FORMAT_D32_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			DXINFO(rs->device->CreateTexture2D(&desc, nullptr, &result.depthMap));
		}

		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			view_desc.Format = TextureFormatToD3D(TextureFormat::R32G32_FLOAT);
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			view_desc.Texture2D.MostDetailedMip = 0;
			view_desc.Texture2D.MipLevels = 1;

			DXINFO(rs->device->CreateShaderResourceView(result.cubeMap, &view_desc, &result.view));
		}

		for (int32 i = 0; i < 6; i++)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtDesc = {};
			rtDesc.Format = TextureFormatToD3D(TextureFormat::R32G32_FLOAT);
			rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtDesc.Texture2DArray.MipSlice = 0;
			rtDesc.Texture2DArray.ArraySize = 1;
			rtDesc.Texture2DArray.FirstArraySlice = i;


			DXCHECK(rs->device->CreateRenderTargetView(result.cubeMap,
				&rtDesc, &result.faceViews[i]));


			D3D11_SHADER_RESOURCE_VIEW_DESC uav = {};
			uav.Format = TextureFormatToD3D(TextureFormat::R32G32_FLOAT);
			uav.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			uav.Texture2DArray.MipLevels = 1;
			uav.Texture2DArray.ArraySize = 1;
			uav.Texture2DArray.FirstArraySlice = i;

			DXINFO(rs->device->CreateShaderResourceView(result.cubeMap, &uav, &result.faceFaceViews[i]));

			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
			depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;//(DXGI_FORMAT)depth_buffer->info.format;
			depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_dsc.Texture2D.MipSlice = 0;
			DXCHECK(rs->device->CreateDepthStencilView(result.depthMap,
				&depth_view_dsc, &result.depthViews[i]));
		}

		return result;
	}

	void BindSampler(RenderState* rs, SamplerInstance* sampler, int32 register_)
	{
		DXINFO(rs->context->PSSetSamplers(register_, 1, &sampler->sampler));
	}

	void BindTexture(RenderState* rs, TextureInstance* texture, ShaderStage stage, int32 register_)
	{
		if (texture)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX: break;
			case ShaderStage::PIXEL:
			{
				DXINFO(rs->context->PSSetShaderResources(register_, 1, &texture->view));
			}break;
			case ShaderStage::COMPUTE:
			{
				DXINFO(rs->context->CSSetShaderResources(register_, 1, &texture->view));
			}break;
			}
		}
		else
		{
			ID3D11ShaderResourceView* nullTexture = nullptr;
			switch (stage)
			{
			case ShaderStage::VERTEX: break;
			case ShaderStage::PIXEL:
			{
				DXINFO(rs->context->PSSetShaderResources(register_, 1, &nullTexture));
			}break;
			case ShaderStage::COMPUTE:
			{
				DXINFO(rs->context->CSSetShaderResources(register_, 1, &nullTexture));
			}break;
			}
		}
	}

	void BindTextureCompute(RenderState* rs, TextureInstance* texture, int32 register_)
	{
		if (texture)
		{
			Assert(texture->uavView, "No underorder access view");
			if (texture->uavView)
			{
				DXINFO(rs->context->CSSetUnorderedAccessViews(register_, 1, &texture->uavView, nullptr));
			}
		}
		else
		{
			ID3D11UnorderedAccessView* nullUAV = nullptr;
			DXINFO(rs->context->CSSetUnorderedAccessViews(register_, 1, &nullUAV, nullptr));
		}
	}

	void BindCubeMap(RenderState* rs, CubeMapInstance* cubemap, int32 register_)
	{
		DXINFO(rs->context->PSSetShaderResources(register_, 1, &cubemap->view));
		DXINFO(rs->context->PSSetSamplers(register_, 1, &cubemap->sampler));
	}

	void BindMesh(RenderState* rs, MeshInstance* mesh)
	{
		uint32 offset = 0;
		DXINFO(rs->context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &mesh->stride_bytes, &offset));
		DXINFO(rs->context->IASetIndexBuffer(mesh->index_buffer, DXGI_FORMAT_R32_UINT, 0));
	}

	void BindInstancedData(RenderState* rs, InstancedData* data)
	{
		uint32 offset = 0;
		if (data)
		{
			DXINFO(rs->context->IASetVertexBuffers(1, 1, &data->buffer, &data->strideBytes, &offset));
		}
		else
		{
			ID3D11Buffer* buffer = { nullptr };
			uint32 nbytes = 0;
			DXINFO(rs->context->IASetVertexBuffers(1, 1, &buffer, &nbytes, &offset));
		}
	}

	void BindShader(RenderState* rs, ShaderInstance* shader)
	{
		DXINFO(rs->context->VSSetShader(shader->vs_shader, nullptr, 0));
		DXINFO(rs->context->PSSetShader(shader->ps_shader, nullptr, 0));
		DXINFO(rs->context->IASetInputLayout(shader->layout));
	}

	void BindComputeShader(RenderState* rs, ComputeShaderInstance* shader)
	{
		DXINFO(rs->context->CSSetShader(shader->cs_shader, nullptr, 0));
	}

	SamplerInstance CreateSamplerInstance(RenderState* rs, const SamplerCreateInfo& cinfo)
	{
		SamplerInstance result = {};

		D3D11_SAMPLER_DESC sam_desc = {};
		sam_desc.Filter = TextureFilterModeToD3D(cinfo.filter);

		sam_desc.AddressU = TextureWrapModeToD3D(cinfo.wrap);
		sam_desc.AddressV = TextureWrapModeToD3D(cinfo.wrap);
		sam_desc.AddressW = TextureWrapModeToD3D(cinfo.wrap);

		DXCHECK(rs->device->CreateSamplerState(&sam_desc, &result.sampler));

		return result;
	}

	SamplerInstance CreateSamplerInstanceComapison(RenderState* rs, const SamplerCreateInfo& cinfo)
	{
		SamplerInstance result = {};

		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS;
		desc.BorderColor[0] = 1.0f;
		desc.BorderColor[1] = 1.0f;
		desc.BorderColor[2] = 1.0f;

		DXCHECK(rs->device->CreateSamplerState(&desc, &result.sampler));

		return result;
	}

	TextureInstance CreateTextureInstance2D(RenderState* rs, const TextureCreateInfo& cinfo)
	{
		Assert(!cinfo.mips, "Todo mips");
		Assert(cinfo.width > 0, "Width is zero");
		Assert(cinfo.height > 0, "Width is zero");
		Assert(cinfo.usage[0] != TextureUsage::NONE, "Must have at least one usage flag !!");

		LOGTODO("mips");
		LOGTODO("samples?");

		TextureInstance result = {};

		int32 bind_flags = 0;
		for (int32 i = 0; i < ArrayCount(cinfo.usage); i++)
		{
			bind_flags |= TextureUsageToD3DBindFlags(cinfo.usage[i]);
		}

		// @NOTE: create the raw texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = cinfo.width;
		desc.Height = cinfo.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = TextureFormatToD3D(cinfo.format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = bind_flags;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (cinfo.pixels)
		{
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = cinfo.pixels;
			sd.SysMemPitch = cinfo.width * sizeof(uint8) * cinfo.channels;
			DXCHECK(rs->device->CreateTexture2D(&desc, &sd, &result.texture));
		}
		else
		{
			DXCHECK(rs->device->CreateTexture2D(&desc, NULL, &result.texture));
		}

		Assert(result.texture, "Could not create texture");

		if (result.texture)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			view_desc.Format = desc.Format;
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MostDetailedMip = 0;
			view_desc.Texture2D.MipLevels = 1;

			DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.view));
		}

		if (result.texture && (bind_flags & D3D11_BIND_UNORDERED_ACCESS))
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uav = {};
			uav.Format = desc.Format;
			uav.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uav.Texture2D.MipSlice = 0;

			DXINFO(rs->device->CreateUnorderedAccessView(result.texture, &uav, &result.uavView));
		}

		result.info = cinfo;
		return result;
	}

	//TextureInstance CreateTextureInstance(RenderState* rs, void* pixels, int32 width, int32 height, int32 channels, bool mips)
	//{
	//	Assert(!mips, "Todo mips");
	//	LOGTODO("mips");
	//	LOGTODO("Image format is always r8g8b8a8");

	//	TextureInstance result = {};

	//	// @NOTE: create the raw texture
	//	D3D11_TEXTURE2D_DESC desc = {};
	//	desc.Width = width;
	//	desc.Height = height;
	//	desc.MipLevels = 1;
	//	desc.ArraySize = 1;
	//	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //DXGI_FORMAT_B8G8R8A8_UNORM;
	//	desc.SampleDesc.Count = 1;
	//	desc.SampleDesc.Quality = 0;
	//	desc.Usage = D3D11_USAGE_DEFAULT;
	//	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//	desc.CPUAccessFlags = 0;
	//	desc.MiscFlags = 0;

	//	D3D11_SUBRESOURCE_DATA sd = {};
	//	sd.pSysMem = pixels;
	//	sd.SysMemPitch = width * sizeof(uint8) * channels;

	//	DXCHECK(rs->device->CreateTexture2D(&desc, &sd, &result.texture));

	//	// @NOTE: Create the shader resource view
	//	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
	//	view_desc.Format = desc.Format;
	//	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//	view_desc.Texture2D.MostDetailedMip = 0;
	//	view_desc.Texture2D.MipLevels = 1;

	//	DXCHECK(rs->device->CreateShaderResourceView(result.texture, &view_desc, &result.view));

	//	// @NOTE: Create a sampler for the texture
	//	D3D11_SAMPLER_DESC sam_desc = {};
	//	sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//	sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	//	sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//	sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	//	DXCHECK(rs->device->CreateSamplerState(&sam_desc, &result.sampler));

	//	return result;
	//}


	CubeMapInstance CreateCubeMapInstance(RenderState* rs, uint8* sides[6], int32 width, int32 height, int32 channels)
	{
		Assert(channels == 4, "Channel cound need to be 4 for DXGI_FORMAT_R8G8B8A8_UNORM");

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 6;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;


		D3D11_SUBRESOURCE_DATA sd[6] = {};
		for (int32 i = 0; i < 6; i++)
		{
			sd[i].pSysMem = sides[i];
			sd[i].SysMemPitch = width * sizeof(uint8) * channels;
		}

		CubeMapInstance result = {};
		DXCHECK(rs->device->CreateTexture2D(&desc, sd, &result.cubemap));

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.Format = desc.Format;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		view_desc.Texture2D.MostDetailedMip = 0;
		view_desc.Texture2D.MipLevels = 1;

		DXCHECK(rs->device->CreateShaderResourceView(result.cubemap, &view_desc, &result.view));

		D3D11_SAMPLER_DESC sam_desc = {};
		sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		DXCHECK(rs->device->CreateSamplerState(&sam_desc, &result.sampler));

		return result;
	}

	void UpdateInstanceData(RenderState* rs, InstancedData* instancedData, void* data, int32 sizeBytes)
	{
		D3D11_MAPPED_SUBRESOURCE resource = {};
		DXCHECK(rs->context->Map(instancedData->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));

		memcpy(resource.pData, data, sizeBytes);
		DXINFO(rs->context->Unmap(instancedData->buffer, 0));
	}

	InstancedData CreateInstanceData(RenderState* rs, int32 sizeBytes, int32 strideBytes)
	{
		InstancedData result = {};
		result.sizeBytes = sizeBytes;
		result.strideBytes = strideBytes;

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.ByteWidth = sizeBytes;
		desc.StructureByteStride = strideBytes;

		DXCHECK(rs->device->CreateBuffer(&desc, nullptr, &result.buffer));

		return result;
	}

	SceneVertexBuffer CreateSceneBuffer(RenderState* rs, int32 boxCount)
	{
		SceneVertexBuffer result = {};
		result.boxCapcity = boxCount;

		uint32 strideBytes = 4 * sizeof(Vec4f);

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.ByteWidth = boxCount * 4 * sizeof(Vec4f);
		desc.StructureByteStride = strideBytes;

		DXCHECK(rs->device->CreateBuffer(&desc, nullptr, &result.buffer));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewDesc.ViewDimension = D3D_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		//viewDesc.Buffer.ElementOffset = sizeof(Vec4f);
		viewDesc.Buffer.NumElements = boxCount;
		//viewDesc.Buffer.ElementWidth = strideBytes; //sizeof(Vec4f);

		DXCHECK(rs->device->CreateShaderResourceView(result.buffer, &viewDesc, &result.view));

		return result;
	}

	MeshInstance CreateMeshInstance(RenderState* rs, real32* vertices, int32 vertex_count, uint32* indices, int32 indices_count)
	{
		static bool once = true;
		if (once)
		{
			once = false;
			LOGTODO("More formats + different strides");
		}
		// @TODO: More formats + different strides
		uint32 vertex_stride_bytes = sizeof(real32) * 3 + sizeof(real32) * 3 + sizeof(real32) * 2;
		uint32 indices_stride_bytes = sizeof(uint32);

		D3D11_BUFFER_DESC vertex_desc = {};
		vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertex_desc.Usage = D3D11_USAGE_DEFAULT;
		vertex_desc.CPUAccessFlags = 0;
		vertex_desc.MiscFlags = 0;
		vertex_desc.ByteWidth = vertex_count * sizeof(real32);
		vertex_desc.StructureByteStride = vertex_stride_bytes;

		D3D11_SUBRESOURCE_DATA vertex_res = {};
		vertex_res.pSysMem = vertices;

		D3D11_BUFFER_DESC index_desc = {};
		index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		index_desc.Usage = D3D11_USAGE_DEFAULT;
		index_desc.CPUAccessFlags = 0;
		index_desc.MiscFlags = 0;
		index_desc.ByteWidth = indices_count * sizeof(uint32);
		index_desc.StructureByteStride = sizeof(uint32);

		D3D11_SUBRESOURCE_DATA index_res = {};
		index_res.pSysMem = indices;

		MeshInstance result = {  };
		DXCHECK(rs->device->CreateBuffer(&vertex_desc, &vertex_res, &result.vertex_buffer));
		DXCHECK(rs->device->CreateBuffer(&index_desc, &index_res, &result.index_buffer));
		result.stride_bytes = vertex_stride_bytes;
		result.index_count = indices_count;

		return result;
	}


	void BindRenderTarget(RenderState* rs, RenderTarget* rt)
	{
		if (rt)
		{
			int32 count = 0;
			for (int32 i = 0; i < ArrayCount(rt->renderTargets); i++)
			{
				if (rt->renderTargets[i])
					count++;
			}

			if (count > 0)
			{
				DXINFO(rs->context->OMSetRenderTargets(count, rt->renderTargets, rt->depth_target));
			}
		}
		else
		{
			ID3D11RenderTargetView* nullTarget[4] = { nullptr, nullptr, nullptr, nullptr };
			DXINFO(rs->context->OMSetRenderTargets(4, nullTarget, nullptr));
		}
	}

	void ClearRenderTarget(RenderState* rs, RenderTarget* rt, const Vec4f& colour)
	{
		if (rt->renderTarget0)
		{
			for (int32 i = 0; i < ArrayCount(rt->renderTargets); i++)
			{
				if (rt->renderTargets[i])
				{
					DXINFO(rs->context->ClearRenderTargetView(rt->renderTargets[i], colour.ptr));
				}
			}
		}

		if (rt->depth_target)
		{
			DXINFO(rs->context->ClearDepthStencilView(rt->depth_target, D3D11_CLEAR_DEPTH, 1.0f, 0))
		}
	}

	RenderTarget CreateRenderTarget(RenderState* rs, TextureInstance* colour_buffer, bool32 depth)
	{
		RenderTarget result = {};

		if (colour_buffer->texture)
		{
			D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
			render_target_desc.Format = TextureFormatToD3D(colour_buffer->info.format);
			render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			render_target_desc.Texture2D.MipSlice = 0;

			DXCHECK(rs->device->CreateRenderTargetView(colour_buffer->texture,
				&render_target_desc, &result.renderTarget0));

			result.colourTexture0 = *colour_buffer;

			result.width = colour_buffer->info.width;
			result.height = colour_buffer->info.height;
		}

		if (depth)
		{
			// TODO: Prehaps we just take a depth buffer create info if we don't need more functioanllity ?
			LOGTODO("Depth buffer params");

			// @NOTE: Create depth buffer
			D3D11_TEXTURE2D_DESC depth_ds = {};
			depth_ds.Width = colour_buffer->info.width;
			depth_ds.Height = colour_buffer->info.height;
			depth_ds.MipLevels = 1;
			depth_ds.ArraySize = 1;
			depth_ds.Format = DXGI_FORMAT_D32_FLOAT;
			depth_ds.SampleDesc.Count = 1;
			depth_ds.SampleDesc.Quality = 0;
			depth_ds.Usage = D3D11_USAGE_DEFAULT;
			depth_ds.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			DXCHECK(rs->device->CreateTexture2D(&depth_ds, nullptr, &result.depth_texture));

			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
			depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;//(DXGI_FORMAT)depth_buffer->info.format;
			depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_dsc.Texture2D.MipSlice = 0;
			DXCHECK(rs->device->CreateDepthStencilView(result.depth_texture,
				&depth_view_dsc, &result.depth_target));
		}

		return result;
	}

	RenderTarget CreateRenderTarget(RenderState* rs, int32 width, int32 height, TextureFormat format)
	{
		RenderTarget render_target = {};

		// @NOTE: Create colour texture 
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width = width;
		texture_desc.Height = height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = TextureFormatToD3D(format);
		texture_desc.SampleDesc.Count = 1;
		texture_desc.SampleDesc.Quality = 0;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.CPUAccessFlags = 0;
		texture_desc.MiscFlags = 0;

		DXCHECK(rs->device->CreateTexture2D(&texture_desc, NULL, &render_target.colourTexture0.texture));

		// @NOTE: Create view for colour texture
		D3D11_SHADER_RESOURCE_VIEW_DESC shader_view_desc = {};
		shader_view_desc.Format = texture_desc.Format;
		shader_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_view_desc.Texture2D.MostDetailedMip = 0;
		shader_view_desc.Texture2D.MipLevels = 1;

		DXCHECK(rs->device->CreateShaderResourceView(render_target.colourTexture0.texture,
			&shader_view_desc, &render_target.colourTexture0.view));

		// ----------------------------------------------------------------------------
		// ----------------------------------------------------------------------------

		D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
		render_target_desc.Format = texture_desc.Format;
		render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		render_target_desc.Texture2D.MipSlice = 0;

		DXCHECK(rs->device->CreateRenderTargetView(render_target.colourTexture0.texture,
			&render_target_desc, &render_target.renderTarget0));

		return render_target;

		//if (depth_buffer)
		//{
		//	D3D11_TEXTURE2D_DESC depth_ds = {};
		//	depth_ds.Width = width;
		//	depth_ds.Height = height;
		//	depth_ds.MipLevels = 1;
		//	depth_ds.ArraySize = 1;
		//	depth_ds.Format = DXGI_FORMAT_D32_FLOAT;
		//	depth_ds.SampleDesc.Count = gc->msaa_sample_count;
		//	depth_ds.SampleDesc.Quality = 0;
		//	depth_ds.Usage = D3D11_USAGE_DEFAULT;
		//	depth_ds.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		//	gc->device->CreateTexture2D(&depth_ds, nullptr, &depth_texture);

		//	D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
		//	depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
		//	depth_view_dsc.ViewDimension = gc->IsMSAAEnabled() ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		//	depth_view_dsc.Texture2D.MipSlice = 0;
		//	DXCHECK(gc->device->CreateDepthStencilView(depth_texture, &depth_view_dsc, &depth_target));
		//}
	}

	void AddTextureToRenderTarget(RenderState* rs, RenderTarget* renderTarget, TextureInstance* texture)
	{
		Assert(texture->info.width == renderTarget->width, "Render tarets dimensions are different");
		Assert(texture->info.height == renderTarget->height, "Render tarets dimensions are different");

		D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
		render_target_desc.Format = TextureFormatToD3D(texture->info.format);
		render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		render_target_desc.Texture2D.MipSlice = 0;

		for (int32 i = 0; i < ArrayCount(renderTarget->renderTargets); i++)
		{
			if (!renderTarget->renderTargets[i])
			{
				DXCHECK(rs->device->CreateRenderTargetView(texture->texture,
					&render_target_desc, &renderTarget->renderTargets[i]));

				renderTarget->colourTextures[i] = *texture;
				break;
			}
		}
	}

	RenderTarget CreateMutliSampleRenderTarget(RenderState* rs, int32 width, int32 height, int32 samples)
	{
		RenderTarget render_target = {};

		// @NOTE: Create colour texture 
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width = width;
		texture_desc.Height = height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texture_desc.SampleDesc.Count = samples;
		texture_desc.SampleDesc.Quality = 0;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.CPUAccessFlags = 0;
		texture_desc.MiscFlags = 0;

		DXCHECK(rs->device->CreateTexture2D(&texture_desc, NULL, &render_target.colourTexture0.texture));

		// @NOTE: Create view for colour texture
		D3D11_SHADER_RESOURCE_VIEW_DESC shader_view_desc = {};
		shader_view_desc.Format = texture_desc.Format;
		shader_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		shader_view_desc.Texture2D.MostDetailedMip = 0;
		shader_view_desc.Texture2D.MipLevels = 1;

		DXCHECK(rs->device->CreateShaderResourceView(render_target.colourTexture0.texture,
			&shader_view_desc, &render_target.colourTexture0.view));

		// ----------------------------------------------------------------------------
		// ----------------------------------------------------------------------------

		// @NOTE: Create depth buffer
		D3D11_TEXTURE2D_DESC depth_ds = {};
		depth_ds.Width = width;
		depth_ds.Height = height;
		depth_ds.MipLevels = 1;
		depth_ds.ArraySize = 1;
		depth_ds.Format = DXGI_FORMAT_D32_FLOAT;
		depth_ds.SampleDesc.Count = samples;
		depth_ds.SampleDesc.Quality = 0;
		depth_ds.Usage = D3D11_USAGE_DEFAULT;
		depth_ds.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		DXCHECK(rs->device->CreateTexture2D(&depth_ds, nullptr, &render_target.depth_texture));

		// ----------------------------------------------------------------------------
		// ----------------------------------------------------------------------------

		// @NOTE: Create render target for colour texture
		D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = {};
		render_target_desc.Format = texture_desc.Format;
		render_target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		render_target_desc.Texture2D.MipSlice = 0;

		DXCHECK(rs->device->CreateRenderTargetView(render_target.colourTexture0.texture,
			&render_target_desc, &render_target.renderTarget0));

		// @NOTE: Create render target for depth texture
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_dsc = {};
		depth_view_dsc.Format = DXGI_FORMAT_D32_FLOAT;
		depth_view_dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depth_view_dsc.Texture2D.MipSlice = 0;
		DXCHECK(rs->device->CreateDepthStencilView(render_target.depth_texture,
			&depth_view_dsc, &render_target.depth_target));


		return render_target;
	}

	ShaderBuffer CreateShaderBuffer(RenderState* rs, int32 size_bytes)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.ByteWidth = size_bytes;
		desc.StructureByteStride = 0;

		ShaderBuffer result = {};

		DXCHECK(rs->device->CreateBuffer(&desc, NULL, &result.buffer));
		result.size_bytes = size_bytes;
		result.copy_ptr = 0;
		Assert(ArrayCount(result.staging_buffer) > size_bytes, "Shader buffer staging buffer is not big enough");

		return result;
	}

	void CopyVec4iIntoShaderBuffer(ShaderBuffer* buffer, const Vec4i& a)
	{
		Assert(buffer->copy_ptr + 4 <= buffer->size_bytes / 4, "DXConstBuffer::CopyInVec3o buffer overrun");

		int32* ptr = (int32*)buffer->staging_buffer;

		ptr[buffer->copy_ptr] = a.x;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = a.y;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = a.z;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = 0;
		buffer->copy_ptr++;
	}

	void CopyVec3fIntoShaderBuffer(ShaderBuffer* buffer, const Vec3f& a)
	{
		Assert(buffer->copy_ptr + 4 <= buffer->size_bytes / 4, "DXConstBuffer::CopyInVec3f buffer overrun");

		real32* ptr = (real32*)buffer->staging_buffer;

		ptr[buffer->copy_ptr] = a.x;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = a.y;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = a.z;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = 0.0f;
		buffer->copy_ptr++;
	}

	void CopyVec4fIntoShaderBuffer(ShaderBuffer* buffer, const Vec4f& a)
	{
		Assert(buffer->copy_ptr + 4 <= buffer->size_bytes / 4, "DXConstBuffer::CopyInVec3f buffer overrun");

		real32* ptr = (real32*)buffer->staging_buffer;

		ptr[buffer->copy_ptr] = a.x;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = a.y;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = a.z;
		buffer->copy_ptr++;
		ptr[buffer->copy_ptr] = a.w;
		buffer->copy_ptr++;
	}

	void CopyMat4fIntoShaderBuffer(ShaderBuffer* buffer, const Mat4f& a, bool32 transpose)
	{
		Assert(buffer->copy_ptr + 16 <= buffer->size_bytes / 4, "DXConstBuffer::CopyInMat4f buffer overrun");

		real32* ptr = (real32*)buffer->staging_buffer;

		// @TODO: There is probably a faster way, without the call to transopse, but that is way to premature to optimze
		Mat4f aT = Transpose(a);
		for (int32 i = 0; i < 16; i++)
		{
			ptr[buffer->copy_ptr] = aT.ptr[i];
			buffer->copy_ptr++;
		}
	}

	void UpdateShaderBuffer(RenderState* rs, ShaderBuffer* buffer)
	{
		DXINFO(rs->context->UpdateSubresource(buffer->buffer, 0, nullptr, (void*)buffer->staging_buffer, 0, 0));
		ZeroMemory(buffer->staging_buffer, buffer->size_bytes);
		buffer->copy_ptr = 0;
	}

	void BindShaderBuffer(RenderState* rs, ShaderBuffer* buffer, ShaderStage stage, int32 register_)
	{
		switch (stage)
		{
		case ShaderStage::VERTEX:
		{
			DXINFO(rs->context->VSSetConstantBuffers(register_, 1, &buffer->buffer));
		}break;
		case ShaderStage::PIXEL:
		{
			DXINFO(rs->context->PSSetConstantBuffers(register_, 1, &buffer->buffer));
		}break;
		case ShaderStage::COMPUTE:
		{
			DXINFO(rs->context->CSSetConstantBuffers(register_, 1, &buffer->buffer));
		}break;
		}
	}

	void FreeShader(ShaderInstance* shader)
	{
		Assert(shader, "Shader was null!!");
		if (shader)
		{
			DXRELEASE(shader->vs_shader);
			DXRELEASE(shader->ps_shader);
			DXRELEASE(shader->layout);
		}
	}

	void RenderScreenSpaceQuad(RenderState* rs)
	{
		BindMesh(rs, &rs->screen_space_quad);
		DXINFO(rs->context->DrawIndexed(rs->screen_space_quad.index_count, 0, 0));
	}

	void InitializeRenderState(RenderState* rs, AssetState* as, PlatformState* ws, TransientState* ts)
	{
		MemoryArena* permanentMemory = GameMemory::GetPermanentMemoryArena();
		InitializeDirectX((HWND)ws->window, rs, as);

		rs->bvh.maxPrimCount = 1;
		rs->pointLightFarPlane = 25.0f;
		rs->pointLightNearPlane = 0.1f;
		rs->bloomThreshold = 1.0f;
		rs->bloomSoftness = 0.5f;

		//rs->pointLightShadowSize = 512;
		//rs->shadowAtlasSize = 4096;
		rs->pointLightShadowSize = 1024;
		rs->shadowAtlasSize = 8192;
		rs->pointLightShadowAtlasDelta = (real32)rs->pointLightShadowSize / (real32)rs->shadowAtlasSize;

		rs->msaa_render_target = CreateMutliSampleRenderTarget(rs,
			ws->client_width, ws->client_height, 4);
		rs->post_render_target = CreateRenderTarget(rs,
			ws->client_width, ws->client_height, TextureFormat::R32G32B32A32_FLOAT);

		// @NOTE: Sampler linear
		{
			SamplerCreateInfo cinfo = {};
			cinfo.filter = TextureFilterMode::BILINEAR;
			cinfo.wrap = TextureWrapMode::CLAMP_EDGE;
			rs->linear_sampler = CreateSamplerInstance(rs, cinfo);
		}
		// @NOTE: Sample point
		{
			SamplerCreateInfo cinfo = {};
			cinfo.filter = TextureFilterMode::POINT;
			cinfo.wrap = TextureWrapMode::CLAMP_EDGE;
			rs->point_sampler = CreateSamplerInstance(rs, cinfo);
		}
		// @NOTE: Sample PFC
		{
			SamplerCreateInfo cinfo = {};
			rs->pcfSampler = CreateSamplerInstanceComapison(rs, cinfo);
		}

		// @NOTE: Termpory texture 
		{
			TextureCreateInfo cinfo = {};
			cinfo.width = 4096;
			cinfo.height = 4096;
			cinfo.format = TextureFormat::R32G32B32A32_FLOAT;
			cinfo.usage[0] = TextureUsage::SHADER_RESOURCE;
			cinfo.usage[1] = TextureUsage::RENDER_TARGET;
			cinfo.usage[2] = TextureUsage::COMPUTER_SHADER_RESOURCE;

			rs->temporayTexture = CreateTextureInstance2D(rs, cinfo);
		}

		// @NOTE: GBuffer
		{
			TextureCreateInfo cinfo = {};
			cinfo.width = ws->client_width;
			cinfo.height = ws->client_height;
			cinfo.format = TextureFormat::R32G32B32A32_FLOAT;
			cinfo.usage[0] = TextureUsage::SHADER_RESOURCE;
			cinfo.usage[1] = TextureUsage::RENDER_TARGET;

			TextureInstance positionTexture = CreateTextureInstance2D(rs, cinfo);
			TextureInstance normalTexture = CreateTextureInstance2D(rs, cinfo);
			TextureInstance albedoTexture = CreateTextureInstance2D(rs, cinfo);

			rs->gbufferRenderTarget = CreateRenderTarget(rs, &positionTexture, true);

			AddTextureToRenderTarget(rs, &rs->gbufferRenderTarget, &normalTexture);
			AddTextureToRenderTarget(rs, &rs->gbufferRenderTarget, &albedoTexture);
		}

		// @NOTE: Forward render target
		{
			TextureCreateInfo cinfo = {};
			cinfo.width = ws->client_width;
			cinfo.height = ws->client_height;
			cinfo.format = TextureFormat::R32G32B32A32_FLOAT;
			cinfo.usage[0] = TextureUsage::SHADER_RESOURCE;
			cinfo.usage[1] = TextureUsage::RENDER_TARGET;

			TextureInstance texture = CreateTextureInstance2D(rs, cinfo);

			rs->forwardPassRenderTarget = CreateRenderTarget(rs, &texture, true);
		}

		// @NOTE: Shadow Buffer
		{
			TextureCreateInfo cinfo = {};
			cinfo.width = ws->client_width;
			cinfo.height = ws->client_height;
			cinfo.format = TextureFormat::R32G32B32A32_FLOAT;

			cinfo.usage[0] = TextureUsage::SHADER_RESOURCE;
			//cinfo.usage[1] = TextureUsage::RENDER_TARGET;
			cinfo.usage[2] = TextureUsage::COMPUTER_SHADER_RESOURCE;

			rs->shadowBuffer = CreateTextureInstance2D(rs, cinfo);
		}

		// @NOTE: Bloom filter
		{
			TextureCreateInfo cinfo = {};
			cinfo.width = ws->client_width;
			cinfo.height = ws->client_height;
			cinfo.format = TextureFormat::R32G32B32A32_FLOAT;
			cinfo.usage[0] = TextureUsage::SHADER_RESOURCE;
			cinfo.usage[1] = TextureUsage::RENDER_TARGET;
			cinfo.usage[2] = TextureUsage::COMPUTER_SHADER_RESOURCE;

			TextureInstance texture = CreateTextureInstance2D(rs, cinfo);

			rs->bloomRenderTarget = CreateRenderTarget(rs, &texture, false);
		}

		rs->sceneBuffer = CreateSceneBuffer(rs, 40000);
		rs->particleTransformBuffer = CreateInstanceData(rs,
			MAX_PARTICLES_PER_EMITTER * sizeof(Mat4f), sizeof(Mat4f));

		rs->vs_matrices = CreateShaderBuffer(rs, sizeof(Mat4f) * 5);
		rs->ps_lighting_info = CreateShaderBuffer(rs, sizeof(Vec3f) * 65);
		rs->ps_lighting_constants = CreateShaderBuffer(rs, sizeof(Vec4f) * 2);
		rs->psTransientData = CreateShaderBuffer(rs, sizeof(Vec4i) * 1);

		//rs->pbr_texture = DEBUGCreateTexture(rs, "../Assets/Textures/PolygonScifi_01_C.png");
		rs->pbr_texture = DEBUGCreateTexture(rs, as, "../Assets/Raw/Textures/demo_room.png");
		DEBUGCreateTexture(rs, as, "../Assets/Raw/Textures/bunk-1.png");
		DEBUGCreateTexture(rs, as, "../Assets/Raw/Textures/terminal.png");
		DEBUGCreateTexture(rs, as, "../Assets/Raw/Textures/hallway.png");
		DEBUGCreateTexture(rs, as, "../Assets/Raw/Textures/base.png");

		rs->cube_mesh = DEBUGCreateOBJMesh(rs, as, permanentMemory, "../Assets/Raw/Models/cube.obj");
		rs->demo_room_mesh = DEBUGCreateMesh(rs, as, permanentMemory,
			"../Assets/Raw/Models/200x200FloorTile.obj", "../Assets/Raw/Models/200x200FloorTile.ply");

		//EditorPreprocessAllTextures("../Assets/Textures/");
		//EditorPreprocessAllMeshes("../Assets/Models/");

		EditorPreprocessOutOfDateMeshes("../Assets/Raw/Models/", ts);

		EditorLoadProcessedAssets(rs, as, permanentMemory, "../Assets/Processed/Meshes/");

		//EditorLoadFontFile(rs, "../Assets/Fonts/Arial.fnt");
		//DEBUGCreateAllMeshes(rs, &gs->permanent_memory, "../Assets/Models/");

		rs->pbr_shader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/basic_pbr.vert.cso",
			"../Assets/Processed/Shaders/basic_pbr.pixl.cso");

		int32 skybox_shader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/skybox.vert.cso",
			"../Assets/Processed/Shaders/skybox.pixl.cso");

		rs->pbr_shader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/basic_pbr.vert.cso",
			"../Assets/Processed/Shaders/basic_pbr.pixl.cso");

		rs->post_processing_shader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/post_processing.vert.cso",
			"../Assets/Processed/Shaders/post_processing.pixl.cso");

		rs->unlit_shader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/unlit.vert.cso",
			"../Assets/Processed/Shaders/unlit.pixl.cso");

		rs->phong_shader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/phong.vert.cso",
			"../Assets/Processed/Shaders/phong.pixl.cso");

		//rs->debugShadowVolumeShader = DEBUGCreateShaderFromBinary(rs, as,
		//	VertexShaderLayout::P_PAD,
		//	"../Assets/Processed/Shaders/debug_shadow_volume.vert.cso",
		//	"../Assets/Processed/Shaders/debug_shadow_volume.pixl.cso");

		rs->gbufferShader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/gbuffer.vert.cso",
			"../Assets/Processed/Shaders/gbuffer.pixl.cso");

		rs->shadow_shader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/depth_point_lights.vert.cso",
			"../Assets/Processed/Shaders/depth_point_lights.pixl.cso");

		rs->defferdPhongShader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/defferd_phong.vert.cso",
			"../Assets/Processed/Shaders/defferd_phong.pixl.cso");

		//rs->gaussianBlurShader = DEBUGCreateShaderFromBinary(rs, as,
		//	VertexShaderLayout::PNT,
		//	"../Assets/Processed/Shaders/gaussian_blur.vert.cso",
		//	"../Assets/Processed/Shaders/gaussian_blur.pixl.cso");

		rs->bloomFilterShader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNT,
			"../Assets/Processed/Shaders/bloomFilter.vert.cso",
			"../Assets/Processed/Shaders/bloomFilter.pixl.cso");

		rs->particleShader = DEBUGCreateShaderFromBinary(rs, as,
			VertexShaderLayout::PNTM,
			"../Assets/Processed/Shaders/particle.vert.cso",
			"../Assets/Processed/Shaders/particle.pixl.cso");

		rs->pbr_shader = rs->phong_shader;

		rs->shadowRayShader = DEBUGCreateComputeShaderFromBinary(rs, "../Assets/Processed/Shaders/shadow_ray.comp.cso");
		rs->testSphereGenShader = DEBUGCreateComputeShaderFromBinary(rs, "../Assets/Processed/Shaders/test_sphere_gen.comp.cso");
		rs->gaussianBlurShaderHorizontal = DEBUGCreateComputeShaderFromBinary(rs, "../Assets/Processed/Shaders/gaussian_blur_horizontal.comp.cso");
		rs->gaussianBlurShaderVertical = DEBUGCreateComputeShaderFromBinary(rs, "../Assets/Processed/Shaders/gaussian_blur_vertical.comp.cso");

		rs->skybox = DEBUGCreateCubeMap(rs, "../Assets/Raw/Textures/SpaceBox");
		rs->skybox_shader = rs->shaders[skybox_shader];
		rs->skybox_mesh = rs->meshes[rs->cube_mesh];
	}
	static void UpdateEntityShaderBuffer(RenderState* rs, const Mat4f& model, const Mat4f& view, const Mat4f& projection)
	{
		// Update const buffers
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, model * view * projection);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, projection);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, view);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, model);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, Inverse(model));

		UpdateShaderBuffer(rs, &rs->vs_matrices);
		BindShaderBuffer(rs, &rs->vs_matrices, ShaderStage::VERTEX, 0);
	}

	static void UpdateEntityShaderBuffer(RenderState* rs, Camera* camera, const Mat4f& model)
	{
		Mat4f view = Inverse(camera->transform.CalculateTransformMatrix());
		Mat4f projection = PerspectiveLH(DegToRad(camera->yfov), camera->aspect, camera->near_, camera->far_);
		UpdateEntityShaderBuffer(rs, model, view, projection);
	}

	static void UpdateEntityShaderBuffer(RenderState* rs, Camera* camera, Entity* entity)
	{
		Mat4f model = entity->transform.CalculateTransformMatrix();
		Mat4f view = Inverse(camera->transform.CalculateTransformMatrix());
		Mat4f projection = PerspectiveLH(DegToRad(camera->yfov), camera->aspect, camera->near_, camera->far_);
		UpdateEntityShaderBuffer(rs, model, view, projection);
	}

	static void UpdateEntityShaderBuffer(RenderState* rs, GameState* gs, Camera* camera, Entity* entity)
	{
		Mat4f model = GetEntityWorldTransform(gs, entity).CalculateTransformMatrix();
		Mat4f view = Inverse(camera->transform.CalculateTransformMatrix());
		Mat4f projection = PerspectiveLH(DegToRad(camera->yfov), camera->aspect, camera->near_, camera->far_);
		UpdateEntityShaderBuffer(rs, model, view, projection);
	}

	static void RenderBillboard(RenderState* rs, GameState* gs, PlatformState* win_state, Entity* entity)
	{
		Mat4f model = GetEntityWorldTransform(gs, entity).CalculateTransformMatrix();
		Mat4f view = Inverse(gs->camera.transform.CalculateTransformMatrix());
		real32 aspect = (real32)win_state->client_width / (real32)win_state->client_height;
		Mat4f projection = PerspectiveLH(DegToRad(gs->camera.yfov), aspect, gs->camera.near_, gs->camera.far_);

		Mat4f bill = model * view;
		bill[0][0] = entity->transform.scale.x;
		bill[0][1] = 0.0f;
		bill[0][2] = 0.0f;

		bill[1][0] = 0.0f;
		bill[1][1] = entity->transform.scale.y;
		bill[1][2] = 0.0f;

		bill[2][0] = 0.0f;
		bill[2][1] = 0.0f;
		bill[2][2] = entity->transform.scale.z;

		bill = bill * projection;

		// Update const buffers
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, bill);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, projection);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, view);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, model);
		CopyMat4fIntoShaderBuffer(&rs->vs_matrices, Inverse(model));

		UpdateShaderBuffer(rs, &rs->vs_matrices);
		BindShaderBuffer(rs, &rs->vs_matrices, ShaderStage::VERTEX, 0);

		TextureInstance* texture = LookUpTextureInstance(rs, entity->render.texture);
		ShaderInstance* shader = LookUpShaderInstance(rs, entity->render.shader);

		BindTexture(rs, texture, ShaderStage::PIXEL, 0);
		BindShader(rs, shader);

		RenderScreenSpaceQuad(rs);
	}

	static void BlurTextureInPlace(RenderState* rs, TextureInstance* src, TextureInstance* temp)
	{
		Assert(src->info.width <= temp->info.width, "Dims aren't equal");
		Assert(src->info.height <= temp->info.height, "Dims aren't equal");

		int32 width = src->info.width;
		int32 height = src->info.height;

		int32 blurCount = 1;
		BindTexture(rs, src, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, temp, 0);

		BindComputeShader(rs, &rs->gaussianBlurShaderHorizontal);
		for (int32 i = 0; i < blurCount; i++)
		{
			uint32 numGroupsX = (uint32)ceilf(width / 256.0f);
			DXINFO(rs->context->Dispatch(numGroupsX, height, 1));
		}

		BindTexture(rs, nullptr, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, nullptr, 0);

		BindTexture(rs, temp, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, src, 0);

		BindComputeShader(rs, &rs->gaussianBlurShaderVertical);
		for (int32 i = 0; i < blurCount; i++)
		{
			uint32 numGroupsY = (uint32)ceilf(height / 256.0f);
			DXINFO(rs->context->Dispatch(width, numGroupsY, 1));
		}

		BindTexture(rs, nullptr, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, nullptr, 0);
	}

	static void BlurTextureInTo(RenderState* rs, TextureInstance* src, TextureInstance* dst)
	{
		Assert(src->info.width == dst->info.width, "Dims aren't equal");
		Assert(src->info.height == dst->info.height, "Dims aren't equal");

		int32 width = src->info.width;
		int32 height = src->info.height;

		int32 blurCount = 1;
		BindTexture(rs, src, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, &rs->temporayTexture, 0);

		BindComputeShader(rs, &rs->gaussianBlurShaderHorizontal);
		for (int32 i = 0; i < blurCount; i++)
		{
			uint32 numGroupsX = (uint32)ceilf(width / 256.0f);
			DXINFO(rs->context->Dispatch(numGroupsX, height, 1));
		}

		BindTexture(rs, nullptr, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, nullptr, 0);

		BindTexture(rs, &rs->temporayTexture, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, dst, 0);

		BindComputeShader(rs, &rs->gaussianBlurShaderVertical);
		for (int32 i = 0; i < blurCount; i++)
		{
			uint32 numGroupsY = (uint32)ceilf(height / 256.0f);
			DXINFO(rs->context->Dispatch(width, numGroupsY, 1));
		}

		BindTexture(rs, nullptr, ShaderStage::COMPUTE, 0);
		BindTextureCompute(rs, nullptr, 0);
	}

	struct BVHPackedNode
	{
		Vec4f a;
		Vec4f b;
		Vec4f c;
		Vec4f d;
	};

	static inline PackedBox CreatePackedBox(const OBB& obb)
	{
		PackedBox result = {};

		result.orientation = Mat3ToQuat(obb.basis.mat);
		result.centerPad = Vec4f(obb.center, 0.0f);
		result.extents = Vec4f(obb.extents, 0.0f);

		return result;
	}

	static void UpdateSceneData(RenderState* rs, AssetState* as, MemoryArena* arena, EntityRenderGroup* renderGroup)
	{
#if 1
		//SceneVertexBuffer* scene = &rs->sceneBuffer;

		//TemporayArena tempMem = BeginTemporayMemory(arena);

		//int32 sizeBytes = scene->boxCapcity * sizeof(AABB);
		//AABB* primBoundingBoxes = (AABB*)PushSize(tempMem.arena, sizeBytes);

		//int32 centersSizeBytes = scene->boxCapcity * sizeof(Vec3f);
		//Vec3f* primCenters = (Vec3f*)PushSize(tempMem.arena, centersSizeBytes);

		//int32 primSizeBytes = scene->boxCapcity * sizeof(OBB);
		//OBB* primitives = (OBB*)PushSize(tempMem.arena, primSizeBytes);

		//ZeroMemory(primBoundingBoxes, sizeBytes);
		//ZeroMemory(primCenters, centersSizeBytes);
		//ZeroMemory(primitives, primSizeBytes);

		//int32 boxCount = 0;
		//for (int32 entityIndex = 0; entityIndex < renderGroup->testingCount; entityIndex++)
		//{
		//	MeshData* entityMeshData = LookUpMeshData(as, renderGroup->testingEntities[entityIndex].render.mesh);
		//	Mat4f transform = renderGroup->testingEntities[entityIndex].transform.CalculateTransformMatrix();

		//	for (int32 i = 0; i < entityMeshData->shadowBoxCount; i++)
		//	{
		//		OBB obb = CreateOBB(entityMeshData->shadowBoxes[i]);
		//		obb.mat[3][3] = 1;
		//		obb.mat = obb.mat * transform;

		//		//DEBUGDrawOBB(rs, obb);

		//		AABB box = CreateAABBContainingOBB(obb);
		//		primitives[boxCount] = obb;
		//		primBoundingBoxes[boxCount] = box;
		//		primCenters[boxCount] = obb.center;

		//		boxCount++;
		//		Assert(boxCount < scene->boxCapcity, "To many voxel boxes");
		//	}
		//}

		////for (int32 i = 0; i < boxCount; i++)
		////{
		////	DEBUGDrawAABB(rs, primBoundingBoxes[i]);
		////}

		//if (boxCount > 0)
		//{
		//	BVH* bvh = &rs->bvh;
		//	int maxPrimCount = bvh->maxPrimCount;
		//	ZeroStruct(bvh);
		//	bvh->maxPrimCount = maxPrimCount;
		//	BuildBVH(bvh, primBoundingBoxes, primCenters, boxCount);

		//	//DEBUGDrawBVH(rs, bvh);

		//	int32 bvhDataSizeBytes = bvh->nodeCount * sizeof(BVHPackedNode);
		//	BVHPackedNode* bvhData = (BVHPackedNode*)PushSize(tempMem.arena, bvhDataSizeBytes);
		//	ZeroMemory(bvhData, bvhDataSizeBytes);

		//	std::stack<int32> nodeIndices;
		//	nodeIndices.push(0);

		//	while (nodeIndices.size() > 0)
		//	{
		//		int32 currentIndex = nodeIndices.top();
		//		nodeIndices.pop();

		//		BVHNode* node = &bvh->nodes[currentIndex];
		//		BVHPackedNode* pack = &bvhData[currentIndex];


		//		pack->b.w = nodeIndices.size() == 0 ? -1 : (real32)nodeIndices.top();

		//		if (node->IsLeaf())
		//		{
		//			// Pack the triangle
		//			Assert(node->primCount == 1, "Node");
		//			int32 primIndex = bvh->primitiveIndices[node->firstIndex];

		//			OBB* prim = &primitives[primIndex];
		//			Quatf q = Mat3ToQuat(prim->basis.mat);

		//			pack->a.x = prim->center.x;
		//			pack->a.y = prim->center.y;
		//			pack->a.z = prim->center.z;

		//			pack->b.x = prim->extents.x;
		//			pack->b.y = prim->extents.y;
		//			pack->b.z = prim->extents.z;

		//			pack->d.x = q.x;
		//			pack->d.y = q.y;
		//			pack->d.z = q.z;
		//			pack->d.w = q.w;

		//			// Set is leaf to true
		//			pack->c.w = 1.0f;

		//			pack->a.w = pack->b.w;
		//		}
		//		else
		//		{
		//			// Set box
		//			pack->a.x = node->box.min.x;
		//			pack->a.y = node->box.min.y;
		//			pack->a.z = node->box.min.z;

		//			pack->b.x = node->box.max.x;
		//			pack->b.y = node->box.max.y;
		//			pack->b.z = node->box.max.z;

		//			// Set is leaf to false
		//			pack->c.w = 0.0f;

		//			uint32 left = node->firstIndex;
		//			uint32 right = node->firstIndex + 1;

		//			// Set hit link
		//			pack->a.w = (real32)left;

		//			nodeIndices.push((int32)right);
		//			nodeIndices.push((int32)left);
		//		}
		//	}

		//	D3D11_MAPPED_SUBRESOURCE resource = {};
		//	DXCHECK(rs->context->Map(scene->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
		//	memcpy(resource.pData, (void*)bvhData, bvhDataSizeBytes);
		//	DXINFO(rs->context->Unmap(scene->buffer, 0));

		//	DXINFO(rs->context->CSSetShaderResources(0, 1, &scene->view));
		//}

		//CopyVec4iIntoShaderBuffer(&rs->psTransientData, Vec4i(boxCount, 0, 0, 0));
		//UpdateShaderBuffer(rs, &rs->psTransientData);
		//BindShaderBuffer(rs, &rs->psTransientData, ShaderStage::COMPUTE, 2);

		//EndTemporaryMemory(tempMem);
#endif
	}

	static void RenderGBuffer(RenderState* rs, AssetState* as, EntityRenderGroup* renderGroup)
	{
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 2);
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 3);
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 4);
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 5);

		BindTexture(rs, nullptr, ShaderStage::COMPUTE, 2);
		BindTexture(rs, nullptr, ShaderStage::COMPUTE, 3);
		BindTexture(rs, nullptr, ShaderStage::COMPUTE, 4);

		DXINFO(rs->context->CSSetSamplers(1, 1, &rs->point_sampler.sampler));

		ClearRenderTarget(rs, &rs->gbufferRenderTarget, Vec4f(0));
		BindRenderTarget(rs, &rs->gbufferRenderTarget);
		BindShader(rs, LookUpShaderInstance(rs, rs->gbufferShader));

		for (int32 entityIndex = 0; entityIndex < renderGroup->opaqueEntityCount; entityIndex++)
		{
			Entity* entity = &renderGroup->opaqueEntities[entityIndex];

			TextureInstance* texture = LookUpTextureInstance(rs, entity->render.texture);
			MeshInstance* mesh = LookUpMeshInstance(rs, entity->render.mesh);
			BindMesh(rs, mesh);
			BindTexture(rs, texture, ShaderStage::PIXEL, 0);

			UpdateEntityShaderBuffer(rs, &renderGroup->mainCamera, entity);

			DXINFO(rs->context->DrawIndexed(mesh->index_count, 0, 0));
		}

		BindRenderTarget(rs, nullptr);

		static int32 buildCount = 60;
		if (buildCount == 60)
		{
			//	UpdateSceneData(rs, as, mem, renderGroup);
		}

		BindTexture(rs, &rs->gbufferRenderTarget.colourTexture0, ShaderStage::PIXEL, 2);
		BindTexture(rs, &rs->gbufferRenderTarget.colourTexture1, ShaderStage::PIXEL, 3);
		BindTexture(rs, &rs->gbufferRenderTarget.colourTexture2, ShaderStage::PIXEL, 4);

		BindTexture(rs, &rs->gbufferRenderTarget.colourTexture0, ShaderStage::COMPUTE, 2);
		BindTexture(rs, &rs->gbufferRenderTarget.colourTexture1, ShaderStage::COMPUTE, 3);
		BindTexture(rs, &rs->gbufferRenderTarget.colourTexture2, ShaderStage::COMPUTE, 4);

		// @NOTE: Ray trace the shadow buffer
		BindComputeShader(rs, &rs->shadowRayShader);
		BindTextureCompute(rs, &rs->shadowBuffer, 0);

		int32 xGroupCount = (int32)Ceil((real32)rs->shadowBuffer.info.width / 32.0f);
		int32 yGroupCount = (int32)Ceil((real32)rs->shadowBuffer.info.height / 32.0f);

		//DXINFO(rs->context->Dispatch(xGroupCount, yGroupCount, 1));
		BindTextureCompute(rs, nullptr, 0);

		BindTexture(rs, &rs->shadowBuffer, ShaderStage::PIXEL, 5);
	}

	static void RenderEntitiesRenderGroup(RenderState* rs, AssetState* as, EntityRenderGroup* renderGroup, PlatformState* ws, Input* input)
	{

		DXINFO(rs->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		Camera camera = renderGroup->mainCamera;

		// @NOTE: Lighting constants
		CopyVec4fIntoShaderBuffer(&rs->ps_lighting_constants,
			Vec4f(rs->pointLightNearPlane, rs->pointLightFarPlane,
				rs->pointLightShadowAtlasDelta, rs->bloomThreshold));

		CopyVec4fIntoShaderBuffer(&rs->ps_lighting_constants,
			Vec4f(rs->bloomSoftness, 0, 0, 0));


		UpdateShaderBuffer(rs, &rs->ps_lighting_constants);
		BindShaderBuffer(rs, &rs->ps_lighting_constants, ShaderStage::PIXEL, 1);

		ShaderBuffer* lighting = &rs->ps_lighting_info;
		CopyVec3fIntoShaderBuffer(lighting, camera.transform.position);
		CopyVec4iIntoShaderBuffer(lighting, Vec4i(0, 0, renderGroup->pointLightCount, 0));

		for (int32 i = 0; i < MAX_DIRECTIONAL_LIGHT_COUNT; i++)
		{
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
		}

		for (int32 i = 0; i < MAX_SPOT_LIGHT_COUNT; i++)
		{
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
		}

		for (int32 i = 0; i < MAX_POINT_LIGHT_COUNT; i++)
		{
			Entity* entity = &renderGroup->pointLights[i];

			Transform world = entity->transform;
			CopyVec3fIntoShaderBuffer(lighting, world.position);
			CopyVec3fIntoShaderBuffer(lighting, entity->light.colour * entity->light.intensity);
		}

		UpdateShaderBuffer(rs, lighting);
		BindShaderBuffer(rs, lighting, ShaderStage::PIXEL, 0);
		BindShaderBuffer(rs, lighting, ShaderStage::COMPUTE, 0);

		BindSampler(rs, &rs->linear_sampler, 0);
		BindSampler(rs, &rs->point_sampler, 1);
		BindSampler(rs, &rs->pcfSampler, 2);

		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////

		D3D11_VIEWPORT viewport = {};

		viewport.Width = (real32)ws->client_width;
		viewport.Height = (real32)ws->client_height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		DXINFO(rs->context->RSSetViewports(1, &viewport));
		RenderGBuffer(rs, as, renderGroup);
		ClearRenderTarget(rs, &rs->forwardPassRenderTarget, Vec4f(0, 0, 0, 1.0f));

		DXINFO(rs->context->CopyResource(rs->forwardPassRenderTarget.depth_texture, rs->gbufferRenderTarget.depth_texture));

		BindRenderTarget(rs, &rs->forwardPassRenderTarget);

		DXINFO(rs->context->OMSetDepthStencilState(rs->depthOffState, 1));

		ShaderInstance* defferdShader = LookUpShaderInstance(rs, rs->defferdPhongShader);
		BindShader(rs, defferdShader);
		RenderScreenSpaceQuad(rs);

		DXINFO(rs->context->OMSetDepthStencilState(rs->depthLessEqualState, 1));

		BindCubeMap(rs, &rs->skybox, 0);
		BindShader(rs, &rs->skybox_shader);
		BindMesh(rs, &rs->skybox_mesh);
		DXINFO(rs->context->RSSetState(rs->noFaceCullState));

		Mat4f view = Inverse(camera.transform.CalculateTransformMatrix());
		Mat4f projection = PerspectiveLH(DegToRad(camera.yfov), camera.aspect, camera.near_, camera.far_);
		UpdateEntityShaderBuffer(rs, Mat4f(1), view, projection);
		DXINFO(rs->context->DrawIndexed(rs->skybox_mesh.index_count, 0, 0));

		DEBUGRenderAndFlushDebugDraws(rs);

		DXINFO(rs->context->OMSetDepthStencilState(rs->ds_state, 1));
		DXINFO(rs->context->RSSetState(rs->rs_standard_state));

		for (int32 entityIndex = 0; entityIndex < renderGroup->particleEmitterCount; entityIndex++)
		{
			Entity* entity = &renderGroup->particleEmitters[entityIndex];
			ParticleEmitterPart* emitter = &entity->particlePart;

			MeshInstance* mesh = LookUpMeshInstance(rs, rs->cube_mesh);
			ShaderInstance* shader = LookUpShaderInstance(rs, rs->particleShader);
			BindMesh(rs, mesh);
			BindShader(rs, shader);

			BindInstancedData(rs, nullptr);

			//TemporayArena tempMem = BeginTemporayMemory(transientMemory);


			//int32 particleTranfromCount = 0;
			//Mat4f* particleTranfromData = (Mat4f*)PushSize(tempMem.arena, rs->particleTransformBuffer.sizeBytes);
			//ZeroMemory(particleTranfromData, rs->particleTransformBuffer.sizeBytes);

			//for (int32 particleIndex = 0; particleIndex < MAX_PARTICLES_PER_EMITTER; particleIndex++)
			//{
			//	Particle* particle = &emitter->particles[particleIndex];
			//	if (particle->IsAlive())
			//	{
			//		particleTranfromData[particleTranfromCount] = Transpose(particle->transform.CalculateTransformMatrix());
			//		particleTranfromData[particleTranfromCount][3][0] = particle->colour.x;
			//		particleTranfromData[particleTranfromCount][3][1] = particle->colour.y;
			//		particleTranfromData[particleTranfromCount][3][2] = particle->colour.z;
			//		particleTranfromCount++;
			//	}
			//}


			//UpdateInstanceData(rs, &rs->particleTransformBuffer,
			//	particleTranfromData, particleTranfromCount * sizeof(Mat4f));

			//BindInstancedData(rs, &rs->particleTransformBuffer);

			//DXINFO(rs->context->DrawIndexedInstanced(mesh->index_count, particleTranfromCount, 0, 0, 0));

			//EndTemporaryMemory(tempMem);
		}

		ClearRenderTarget(rs, &rs->bloomRenderTarget, Vec4f(0));
		BindRenderTarget(rs, &rs->bloomRenderTarget);
		ShaderInstance* bloomFilterShader = LookUpShaderInstance(rs, rs->bloomFilterShader);
		BindShader(rs, bloomFilterShader);
		BindTexture(rs, &rs->forwardPassRenderTarget.colourTexture0, ShaderStage::PIXEL, 0);

		RenderScreenSpaceQuad(rs);
		BindRenderTarget(rs, nullptr);
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 0);
		BlurTextureInPlace(rs, &rs->bloomRenderTarget.colourTexture0, &rs->temporayTexture);

		ClearRenderTarget(rs, &rs->swapchain.render_target, Vec4f(0, 0, 0, 1.0f));
		BindRenderTarget(rs, &rs->swapchain.render_target);

		ShaderInstance* post_processing_shader = LookUpShaderInstance(rs, rs->post_processing_shader);
		BindShader(rs, post_processing_shader);

		BindTexture(rs, &rs->forwardPassRenderTarget.colourTexture0, ShaderStage::PIXEL, 0);
		BindTexture(rs, &rs->bloomRenderTarget.colourTexture0, ShaderStage::PIXEL, 1);

		RenderScreenSpaceQuad(rs);

		BindTexture(rs, nullptr, ShaderStage::PIXEL, 0);
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 1);
	}


	static void RenderEntitiesRenderGroupMK2(RenderState* rs, AssetState* as, EntityRenderGroup* renderGroup, PlatformState* ws, Input* input)
	{
		DXINFO(rs->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		Camera camera = renderGroup->mainCamera;

		// @NOTE: Lighting constants
		rs->ps_lighting_constants.CopyVec4fIntoShaderBuffer(
			Vec4f(rs->pointLightNearPlane, rs->pointLightFarPlane, rs->pointLightShadowAtlasDelta, rs->bloomThreshold));

		rs->ps_lighting_constants.CopyVec4fIntoShaderBuffer(
			Vec4f(rs->bloomSoftness, 0, 0, 0));

		UpdateShaderBuffer(rs, &rs->ps_lighting_constants);
		BindShaderBuffer(rs, &rs->ps_lighting_constants, ShaderStage::PIXEL, 1);

		ShaderBuffer* lighting = &rs->ps_lighting_info;
		CopyVec3fIntoShaderBuffer(lighting, camera.transform.position);
		CopyVec4iIntoShaderBuffer(lighting, Vec4i(0, 0, renderGroup->pointLightCount, 0));

		for (int32 i = 0; i < MAX_DIRECTIONAL_LIGHT_COUNT; i++)
		{
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
		}

		for (int32 i = 0; i < MAX_SPOT_LIGHT_COUNT; i++)
		{
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
			CopyVec3fIntoShaderBuffer(lighting, Vec3f(0));
		}

		for (int32 i = 0; i < MAX_POINT_LIGHT_COUNT; i++)
		{
			Entity* entity = &renderGroup->pointLights[i];

			Transform world = entity->transform;
			CopyVec3fIntoShaderBuffer(lighting, world.position);
			CopyVec3fIntoShaderBuffer(lighting, entity->light.colour * entity->light.intensity);
		}

		UpdateShaderBuffer(rs, lighting);
		BindShaderBuffer(rs, lighting, ShaderStage::PIXEL, 0);
		BindShaderBuffer(rs, lighting, ShaderStage::COMPUTE, 0);

		BindSampler(rs, &rs->linear_sampler, 0);
		BindSampler(rs, &rs->point_sampler, 1);
		BindSampler(rs, &rs->pcfSampler, 2);

		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////
		////////////////////////

		D3D11_VIEWPORT viewport = {};

		viewport.Width = (real32)ws->client_width;
		viewport.Height = (real32)ws->client_height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		DXINFO(rs->context->RSSetViewports(1, &viewport));
		RenderGBuffer(rs, as, renderGroup);
		ClearRenderTarget(rs, &rs->forwardPassRenderTarget, Vec4f(0, 0, 0, 1.0f));

		DXINFO(rs->context->CopyResource(rs->forwardPassRenderTarget.depth_texture, rs->gbufferRenderTarget.depth_texture));

		BindRenderTarget(rs, &rs->forwardPassRenderTarget);

		DXINFO(rs->context->OMSetDepthStencilState(rs->depthOffState, 1));

		ShaderInstance* defferdShader = LookUpShaderInstance(rs, rs->defferdPhongShader);
		BindShader(rs, defferdShader);
		RenderScreenSpaceQuad(rs);

		DXINFO(rs->context->OMSetDepthStencilState(rs->depthLessEqualState, 1));

		BindCubeMap(rs, &rs->skybox, 0);
		BindShader(rs, &rs->skybox_shader);
		BindMesh(rs, &rs->skybox_mesh);
		DXINFO(rs->context->RSSetState(rs->noFaceCullState));

		Mat4f view = Inverse(camera.transform.CalculateTransformMatrix());
		Mat4f projection = PerspectiveLH(DegToRad(camera.yfov), camera.aspect, camera.near_, camera.far_);
		UpdateEntityShaderBuffer(rs, Mat4f(1), view, projection);
		DXINFO(rs->context->DrawIndexed(rs->skybox_mesh.index_count, 0, 0));

		DEBUGRenderAndFlushDebugDraws(rs);

		DXINFO(rs->context->OMSetDepthStencilState(rs->ds_state, 1));
		DXINFO(rs->context->RSSetState(rs->rs_standard_state));

		for (int32 entityIndex = 0; entityIndex < renderGroup->particleEmitterCount; entityIndex++)
		{
			Entity* entity = &renderGroup->particleEmitters[entityIndex];
			ParticleEmitterPart* emitter = &entity->particlePart;

			MeshInstance* mesh = LookUpMeshInstance(rs, rs->cube_mesh);
			ShaderInstance* shader = LookUpShaderInstance(rs, rs->particleShader);
			BindMesh(rs, mesh);
			BindShader(rs, shader);

			BindInstancedData(rs, nullptr);
		}

		ClearRenderTarget(rs, &rs->bloomRenderTarget, Vec4f(0));
		BindRenderTarget(rs, &rs->bloomRenderTarget);
		ShaderInstance* bloomFilterShader = LookUpShaderInstance(rs, rs->bloomFilterShader);
		BindShader(rs, bloomFilterShader);
		BindTexture(rs, &rs->forwardPassRenderTarget.colourTexture0, ShaderStage::PIXEL, 0);

		RenderScreenSpaceQuad(rs);
		BindRenderTarget(rs, nullptr);
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 0);
		BlurTextureInPlace(rs, &rs->bloomRenderTarget.colourTexture0, &rs->temporayTexture);

		ClearRenderTarget(rs, &rs->swapchain.render_target, Vec4f(0, 0, 0, 1.0f));
		BindRenderTarget(rs, &rs->swapchain.render_target);

		ShaderInstance* post_processing_shader = LookUpShaderInstance(rs, rs->post_processing_shader);
		BindShader(rs, post_processing_shader);

		BindTexture(rs, &rs->forwardPassRenderTarget.colourTexture0, ShaderStage::PIXEL, 0);
		BindTexture(rs, &rs->bloomRenderTarget.colourTexture0, ShaderStage::PIXEL, 1);

		RenderScreenSpaceQuad(rs);

		BindTexture(rs, nullptr, ShaderStage::PIXEL, 0);
		BindTexture(rs, nullptr, ShaderStage::PIXEL, 1);
	}

	void RenderGame(RenderState* rs, AssetState* as, EntityRenderGroup* renderGroup, PlatformState* ws, Input* input)
	{
		//DEBUGDrawSphere(renderGroup->player.playerPart.collider);
		//DEBUGDrawRay(renderGroup->player.playerPart.groundRay, renderGroup->player.playerPart.groundCheckDist);

		RenderEntitiesRenderGroupMK2(rs, as, renderGroup, ws, input);
	}

	void PresentFrame(RenderState* rs, bool32 vsync)
	{
		DXCHECK(rs->swapchain.swapchain->Present(vsync, 0));
	}
}

#endif