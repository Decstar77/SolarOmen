#include "DX11StaticMesh.h"
#include "DX11Renderer.h"

namespace cm
{

	StaticMesh cm::StaticMesh::Create(const ModelAsset& modelAsset)
	{
		GetRenderState();

		int32 vertexCount = modelAsset.packedCount;
		int32 indexCount = modelAsset.indicesCount;
		uint32* indices = modelAsset.indices;
		real32* vertices = modelAsset.packedVertices;

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

		StaticMesh result = {  };
		DXCHECK(rs->device->CreateBuffer(&vertex_desc, &vertex_res, &result.vertexBuffer));
		DXCHECK(rs->device->CreateBuffer(&index_desc, &index_res, &result.indexBuffer));
		result.strideBytes = vertex_stride_bytes;
		result.indexCount = indexCount;
		result.id = modelAsset.id;

		return result;
	}

	StaticMesh StaticMesh::CreateScreenSpaceQuad()
	{
		real32 vertexData[] = {
	-1, 1, 0,	0, 0, -1,	0, 0,
	1, -1, 0,	0, 0, -1,	1, 1,
	-1, -1, 0,	0, 0, -1,	0, 1,
	1, 1, 0,	0, 0, -1,	1, 0
		};

		uint32 indexData[] = {
			0, 1, 2, 0, 3, 1
		};

		ModelAsset asset = {};
		asset.id = 1;
		asset.packedVertices = vertexData;
		asset.packedCount = ArrayCount(vertexData);
		asset.indices = indexData;
		asset.indicesCount = ArrayCount(indexData);

		return StaticMesh::Create(asset);
	}
}