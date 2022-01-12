#include "DX12Types.h"
#include "d3dx12.h"

namespace sol
{
	StaticTexture StaticTexture::Create(uint32 width, uint32 height, TextureFormat format)
	{
		auto device = RenderState::GetDevice();
		StaticTexture texture = {};

		switch (format.Get())
		{
		case TextureFormat::Value::D32_FLOAT:
		{
			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
			depthOptimizedClearValue.DepthStencil.Stencil = 0;

			auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto resType = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			DXCHECK(device->CreateCommittedResource(
				&heapType,
				D3D12_HEAP_FLAG_NONE,
				&resType,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(&texture.texture)
			));
		} break;
		}


		return texture;
	}
}