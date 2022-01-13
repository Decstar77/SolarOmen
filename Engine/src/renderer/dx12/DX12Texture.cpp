#include "DX12Types.h"
#include "d3dx12.h"

namespace sol
{
	StaticTexture StaticTexture::Create(const TextureResource& textureResource)
	{
		auto device = RenderState::GetDevice();

		StaticTexture result = {};

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Alignment = 0; // @NOTE: DX12 figure it out
		textureDesc.Width = textureResource.width; // width of the texture
		textureDesc.Height = textureResource.height; // height of the texture
		textureDesc.DepthOrArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.Format = GetTextureFormatToD3D(textureResource.format);
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		uint32 imageBytesPerRow = textureResource.width * GetTextureFormatSizeBytes(textureResource.format);
		uint32 totalSize = imageBytesPerRow * textureResource.height;

		auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		DXCHECK(device->CreateCommittedResource(
			&heapType, D3D12_HEAP_FLAG_NONE,
			&textureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, IID_PPV_ARGS(&result.texture)
		));

		uint64 textureUploadBufferSize = 0;
		ID3D12Resource* textureBufferUploadHeap = nullptr;
		DXINFO(device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize));

		heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto headDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);
		DXCHECK(device->CreateCommittedResource(
			&heapType, D3D12_HEAP_FLAG_NONE,
			&headDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&textureBufferUploadHeap)));


		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = textureResource.pixels.data;
		textureData.RowPitch = imageBytesPerRow;
		textureData.SlicePitch = imageBytesPerRow * textureDesc.Height;


		auto commandAllocator = RenderState::GetCurrentCommandAllocator();
		auto commandList = RenderState::GetCommandList();

		DXCHECK(commandAllocator->Reset());
		DXCHECK(commandList->Reset(commandAllocator, NULL));

		UpdateSubresources(commandList, result.texture, textureBufferUploadHeap, 0, 0, 1, &textureData);
		RenderState::ResourceTransition(result.texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		DXCHECK(commandList->Close());

		RenderState::FlushCommandQueue(true);
		RenderState::ExecuteCommandList();
		RenderState::FlushCommandQueue(false);


		DXRELEASE(textureBufferUploadHeap);

		return result;
	}

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
		default: Assert(0, "");
		}


		return texture;
	}
}