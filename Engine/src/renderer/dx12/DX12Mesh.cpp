#include "DX12Types.h"
#include "core/SolarLogging.h"

#include "d3dx12.h"

namespace sol
{
	static ID3D12Resource* CreateResourceHeap(uint32 sizeBytes, D3D12_RESOURCE_STATES state)
	{
		auto vHeapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto vBufferType = CD3DX12_RESOURCE_DESC::Buffer(sizeBytes);
		auto device = RenderState::GetDevice();

		ID3D12Resource* buffer = nullptr;

		DXCHECK(device->CreateCommittedResource(
			&vHeapType, D3D12_HEAP_FLAG_NONE, &vBufferType,
			state, nullptr, IID_PPV_ARGS(&buffer)));

		return buffer;
	}

	static ID3D12Resource* CreateCopyResourceHeap(uint32 sizeBytes)
	{
		auto vHeapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto vBufferType = CD3DX12_RESOURCE_DESC::Buffer(sizeBytes);
		auto device = RenderState::GetDevice();

		ID3D12Resource* buffer = nullptr;

		DXCHECK(device->CreateCommittedResource(
			&vHeapType, D3D12_HEAP_FLAG_NONE, &vBufferType,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&buffer)));

		return buffer;
	}

	StaticMesh StaticMesh::Create(real32* vertices, uint32 vertexCount, VertexLayoutType layout)
	{
		uint32 vertexSizeBytes = vertexCount * layout.GetStride() * sizeof(real32);

		auto vertexBuffer = CreateResourceHeap(vertexSizeBytes, D3D12_RESOURCE_STATE_COPY_DEST);
		auto stagingBuffer = CreateCopyResourceHeap(vertexSizeBytes);

		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = vertices;
		vertexData.RowPitch = vertexSizeBytes;
		vertexData.SlicePitch = vertexSizeBytes;

		auto commandAllocator = RenderState::GetCurrentCommandAllocator();
		auto commandList = RenderState::GetCommandList();

		DXCHECK(commandAllocator->Reset());
		DXCHECK(commandList->Reset(commandAllocator, NULL));

		UpdateSubresources(commandList, vertexBuffer, stagingBuffer, 0, 0, 1, &vertexData);
		RenderState::ResourceTransition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		DXCHECK(commandList->Close());

		RenderState::FlushCommandQueue();
		RenderState::ExecuteCommandList();

		StaticMesh mesh = {};
		mesh.vertexCount = vertexCount;
		mesh.vertexBuffer = vertexBuffer;
		mesh.vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		mesh.vertexBufferView.SizeInBytes = vertexSizeBytes;
		mesh.vertexBufferView.StrideInBytes = layout.GetStride() * sizeof(real32);

		return mesh;
	}
}
