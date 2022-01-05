#include "DX11Types.h"
#include "core/SolarLogging.h"

namespace sol
{
	//void StaticMesh::UpdateVertexBuffer(real32* vertices, uint32 sizeBytes)
	//{
	//	GetRenderState();
	//	D3D11_MAPPED_SUBRESOURCE resource = {};
	//	DXCHECK(rs->context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
	//	memcpy(resource.pData, vertices, sizeBytes);
	//	DXINFO(rs->context->Unmap(vertexBuffer, 0));
	//}

	//StaticMesh StaticMesh::Create(real32* vertices, uint32 sizeBytes, VertexLayoutType layout)
	//{
	//	D3D11_BUFFER_DESC vertexDesc = {};
	//	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//	vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
	//	vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//	vertexDesc.MiscFlags = 0;
	//	vertexDesc.ByteWidth = sizeBytes;
	//	vertexDesc.StructureByteStride = sizeof(real32) * layout.GetStride();

	//	StaticMesh result = {};
	//	result.strideBytes = sizeof(real32) * layout.GetStride();
	//	result.vertexLayout = layout;
	//	GetRenderState();
	//	DXCHECK(rs->device->CreateBuffer(&vertexDesc, nullptr, &result.vertexBuffer));

	//	return result;
	//}

	//StaticMesh cm::StaticMesh::Create(const ModelAsset& modelAsset)
	//{
	//	Assert(modelAsset.layout != VertexLayoutType::Value::INVALID, "Invalid mesh vertex layout");

	//	GetRenderState();

	//	int32 vertexCount = modelAsset.packedVertices.count;
	//	int32 indexCount = modelAsset.indices.count;
	//	uint32* indices = modelAsset.indices.data;
	//	real32* vertices = modelAsset.packedVertices.data;

	//	uint32 vertex_stride_bytes = sizeof(real32) * modelAsset.layout.GetStride();
	//	uint32 indices_stride_bytes = sizeof(uint32);

	//	// @TODO: Look at the IMMUTABLE FLAG ?
	//	D3D11_BUFFER_DESC vertex_desc = {};
	//	vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//	vertex_desc.Usage = D3D11_USAGE_DEFAULT;
	//	vertex_desc.CPUAccessFlags = 0;
	//	vertex_desc.MiscFlags = 0;
	//	vertex_desc.ByteWidth = vertexCount * sizeof(real32);
	//	vertex_desc.StructureByteStride = vertex_stride_bytes;

	//	D3D11_SUBRESOURCE_DATA vertex_res = {};
	//	vertex_res.pSysMem = vertices;

	//	D3D11_BUFFER_DESC index_desc = {};
	//	index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//	index_desc.Usage = D3D11_USAGE_DEFAULT;
	//	index_desc.CPUAccessFlags = 0;
	//	index_desc.MiscFlags = 0;
	//	index_desc.ByteWidth = indexCount * sizeof(uint32);
	//	index_desc.StructureByteStride = sizeof(uint32);

	//	D3D11_SUBRESOURCE_DATA index_res = {};
	//	index_res.pSysMem = indices;

	//	StaticMesh result = {  };
	//	DXCHECK(rs->device->CreateBuffer(&vertex_desc, &vertex_res, &result.vertexBuffer));
	//	DXCHECK(rs->device->CreateBuffer(&index_desc, &index_res, &result.indexBuffer));
	//	result.strideBytes = vertex_stride_bytes;
	//	result.indexCount = indexCount;
	//	result.id = modelAsset.id;
	//	result.vertexLayout = modelAsset.layout;

	//	return result;
	//}

	void StaticMesh::Release(StaticMesh* mesh)
	{
		DXRELEASE(mesh->indexBuffer);
		DXRELEASE(mesh->vertexBuffer);

		GameMemory::ZeroStruct(mesh);
	}

	StaticMesh StaticMesh::Create(real32* vertices, uint32 vertexCount, VertexLayoutType layout)
	{
		D3D11_BUFFER_DESC vertexDesc = {};
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexDesc.MiscFlags = 0;
		vertexDesc.ByteWidth = vertexCount * sizeof(real32);
		vertexDesc.StructureByteStride = layout.GetStride() * sizeof(real32);

		StaticMesh result = {};
		result.vertexLayout = layout;
		result.vertexCount = vertexCount / layout.GetStride();
		result.strideBytes = layout.GetStride() * sizeof(real32);

		DeviceContext dc = GetDeviceContext();
		DXCHECK(dc.device->CreateBuffer(&vertexDesc, nullptr, &result.vertexBuffer));

		SOLTRACE(String("Created static mesh, ").Add(vertexCount * sizeof(real32)).Add(" bytes").GetCStr());

		return result;
	}

	StaticMesh StaticMesh::Create(real32* vertices, uint32 vertexCount, uint32* indices, uint32 indexCount, VertexLayoutType layout)
	{
		uint32 vertexStrideBytes = sizeof(real32) * layout.GetStride();
		uint32 indicesStrideBytes = sizeof(uint32);

		// @TODO: Look at the IMMUTABLE FLAG ?
		D3D11_BUFFER_DESC vertex_desc = {};
		vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertex_desc.Usage = D3D11_USAGE_DEFAULT;
		vertex_desc.CPUAccessFlags = 0;
		vertex_desc.MiscFlags = 0;
		vertex_desc.ByteWidth = vertexCount * sizeof(real32);
		vertex_desc.StructureByteStride = vertexStrideBytes;

		D3D11_SUBRESOURCE_DATA vertex_res = {};
		vertex_res.pSysMem = vertices;

		D3D11_BUFFER_DESC index_desc = {};
		index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		index_desc.Usage = D3D11_USAGE_DEFAULT;
		index_desc.CPUAccessFlags = 0;
		index_desc.MiscFlags = 0;
		index_desc.ByteWidth = indexCount * sizeof(uint32);
		index_desc.StructureByteStride = vertexStrideBytes;

		D3D11_SUBRESOURCE_DATA index_res = {};
		index_res.pSysMem = indices;

		StaticMesh result = {};
		result.strideBytes = vertexStrideBytes;
		result.vertexCount = vertexCount / layout.GetStride();
		result.indexCount = indexCount;
		result.vertexLayout = layout;

		DeviceContext dc = GetDeviceContext();

		DXCHECK(dc.device->CreateBuffer(&vertex_desc, &vertex_res, &result.vertexBuffer));
		DXCHECK(dc.device->CreateBuffer(&index_desc, &index_res, &result.indexBuffer));

		SOLTRACE(String("Created static mesh: ").Add(vertexCount * sizeof(real32) + indexCount * sizeof(uint32)).Add(" bytes").GetCStr());

		//SOLFATAL(String("Unsupported mesh format").Add(layout.ToString()).GetCStr());
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

		return StaticMesh::Create(vertexData, ArrayCount(vertexData), indexData, ArrayCount(indexData), VertexLayoutType::Value::PNT);
	}

	StaticMesh StaticMesh::CreateUnitCube()
	{
		real32 vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};

		return StaticMesh::Create(vertices, ArrayCount(vertices), VertexLayoutType::Value::PNT);
	}
}