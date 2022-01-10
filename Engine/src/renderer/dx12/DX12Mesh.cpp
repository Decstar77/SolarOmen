#include "DX12Types.h"
#include "core/SolarLogging.h"

#include "d3dx12.h"

namespace sol
{
	static ID3D12Resource* CreateStoredResourceHeap(uint32 sizeBytes)
	{
		auto vHeapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto vBufferType = CD3DX12_RESOURCE_DESC::Buffer(sizeBytes);
		auto device = GetDevice();

		ID3D12Resource* buffer = nullptr;

		DXCHECK(device->CreateCommittedResource(
			&vHeapType,
			D3D12_HEAP_FLAG_NONE,
			&vBufferType,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer)));
	}

	StaticMesh StaticMesh::Create(ID3D12CommandQueue* queue, real32* vertices, uint32 vertexCount, VertexLayoutType layout)
	{




		// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
		//vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

		//vHeapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		//ID3D12Resource* vBufferUploadHeap;
		//DXCHECK(device->CreateCommittedResource(
		//	&vHeapType,
		//	D3D12_HEAP_FLAG_NONE,
		//	&vBufferType,
		//	D3D12_RESOURCE_STATE_GENERIC_READ,
		//	nullptr,
		//	IID_PPV_ARGS(&vBufferUploadHeap)));
		//vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

		//D3D12_SUBRESOURCE_DATA vertexData = {};
		//vertexData.pData = reinterpret_cast<BYTE*>(vList);
		//vertexData.RowPitch = vBufferSize;
		//vertexData.SlicePitch = vBufferSize;

		//DXCHECK(commandAllocator[frameIndex]->Reset());
		//DXCHECK(commandList->Reset(commandAllocator[frameIndex], NULL));

		//UpdateSubresources(commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

		//auto transition = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		//commandList->ResourceBarrier(1, &transition);

		return {};
	}
}
