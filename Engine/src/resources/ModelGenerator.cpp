#include "SolarResourceTypes.h"
#include "core/SolarLogging.h"

namespace sol
{
	struct Vertex
	{
		Vec3f position;
		Vec3f normal;
		Vec3f tanget;
		Vec2f uv;

		Vertex() {}
		Vertex(
			real32 px, real32 py, real32 pz,
			real32 nx, real32 ny, real32 nz,
			real32 tx, real32 ty, real32 tz,
			real32 u, real32 v)
		{
			position = Vec3f(px, py, pz);
			normal = Vec3f(nx, ny, nz);
			tanget = Vec3f(tx, ty, tz);
			uv = Vec2f(u, v);
		}
	};

	struct MeshData
	{
		ManagedArray<Vertex> vertices;
		ManagedArray<uint32> indices;
	};

	static Vertex ComputeMidPoint(const Vertex& a, const Vertex& b)
	{
		Vertex result = {};
		result.position = 0.5f * (a.position + b.position);
		result.normal = Normalize(0.5f * (a.normal + b.normal));
		result.tanget = Normalize(0.5f * (a.tanget + b.tanget));
		result.uv = 0.5f * (a.uv + b.uv);

		return result;
	}

	static MeshData Subdivide(MeshData* mesh)
	{
		uint32 numTris = (uint32)mesh->indices.count / 3;

		MeshData result = {};
		result.vertices.Allocate(numTris * 6, MemoryType::TRANSIENT);
		result.indices.Allocate(numTris * 12, MemoryType::TRANSIENT);

		//       v1
		//       *
		//      / \
		//     /   \
		//  m0*-----*m1
		//   / \   / \
		//  /   \ /   \
		// *-----*-----*
		// v0    m2     v2

		for (uint32 i = 0; i < numTris; ++i)
		{
			Vertex v0 = mesh->vertices[mesh->indices[i * 3 + 0]];
			Vertex v1 = mesh->vertices[mesh->indices[i * 3 + 1]];
			Vertex v2 = mesh->vertices[mesh->indices[i * 3 + 2]];

			Vertex m0 = ComputeMidPoint(v0, v1);
			Vertex m1 = ComputeMidPoint(v1, v2);
			Vertex m2 = ComputeMidPoint(v0, v2);

			result.vertices.Add(v0); // 0
			result.vertices.Add(v1); // 1
			result.vertices.Add(v2); // 2
			result.vertices.Add(m0); // 3
			result.vertices.Add(m1); // 4
			result.vertices.Add(m2); // 5

			result.indices.Add(i * 6 + 0);
			result.indices.Add(i * 6 + 3);
			result.indices.Add(i * 6 + 5);

			result.indices.Add(i * 6 + 3);
			result.indices.Add(i * 6 + 4);
			result.indices.Add(i * 6 + 5);

			result.indices.Add(i * 6 + 5);
			result.indices.Add(i * 6 + 4);
			result.indices.Add(i * 6 + 2);

			result.indices.Add(i * 6 + 3);
			result.indices.Add(i * 6 + 1);
			result.indices.Add(i * 6 + 4);
		}

		return result;
	}

	ModelResource ConvertMeshDataIntoModelResource(MeshData* meshData, VertexLayoutType layout)
	{
		ModelResource result = {};

		result.layout = layout;
		switch (layout.Get())
		{
		case VertexLayoutType::Value::PNT:
		{
			result.indices = meshData->indices;
			result.packedVertices.Allocate(meshData->vertices.count * layout.GetStride(), MemoryType::TRANSIENT);

			for (uint32 i = 0; i < meshData->vertices.count; i++)
			{
				result.packedVertices.Add(meshData->vertices[i].position.x);
				result.packedVertices.Add(meshData->vertices[i].position.y);
				result.packedVertices.Add(meshData->vertices[i].position.z);

				result.packedVertices.Add(meshData->vertices[i].normal.x);
				result.packedVertices.Add(meshData->vertices[i].normal.y);
				result.packedVertices.Add(meshData->vertices[i].normal.z);

				result.packedVertices.Add(meshData->vertices[i].uv.x);
				result.packedVertices.Add(meshData->vertices[i].uv.y);
			}
		} break;
		default: Assert(0, "Unsupported vertex layout type");
		}

		return result;
	}

	ModelResource ModelGenerator::CreateQuad(real32 x, real32 y, real32 w, real32 h, real32 depth)
	{
		MeshData meshData = {};

		meshData.vertices.Allocate(4, MemoryType::TRANSIENT);
		meshData.indices.Allocate(6, MemoryType::TRANSIENT);

		// Position coordinates specified in NDC space.
		meshData.vertices[0] = Vertex(x, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		meshData.vertices[1] = Vertex(x, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		meshData.vertices[2] = Vertex(x + w, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		meshData.vertices[3] = Vertex(x + w, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		meshData.indices[0] = 0;
		meshData.indices[1] = 1;
		meshData.indices[2] = 2;
		meshData.indices[3] = 0;
		meshData.indices[4] = 2;
		meshData.indices[5] = 3;

		meshData.vertices.count = meshData.vertices.capcity;
		meshData.indices.count = meshData.indices.capcity;

		return ConvertMeshDataIntoModelResource(&meshData, VertexLayoutType::Value::PNT);
	}

	ModelResource ModelGenerator::CreateBox(real32 width, real32 height, real32 depth, uint32 numSubdivisions, VertexLayoutType layout)
	{
		//ModelResource model = {};
		//model.id.number = 3;
		//model.name = "Box";

		MeshData data = {};
		data.vertices.Allocate(24, MemoryType::TRANSIENT);
		data.indices.Allocate(36, MemoryType::TRANSIENT);

		real32 w2 = 0.5f * width;
		real32 h2 = 0.5f * height;
		real32 d2 = 0.5f * depth;

		// Fill in the front face vertex data.
		data.vertices[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		data.vertices[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		data.vertices[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		data.vertices[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// Fill in the back face vertex data.
		data.vertices[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		data.vertices[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		data.vertices[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		data.vertices[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

		// Fill in the top face vertex data.
		data.vertices[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		data.vertices[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		data.vertices[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		data.vertices[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// Fill in the bottom face vertex data.
		data.vertices[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		data.vertices[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		data.vertices[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		data.vertices[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

		// Fill in the left face vertex data.
		data.vertices[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
		data.vertices[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		data.vertices[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
		data.vertices[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

		// Fill in the right face vertex data.
		data.vertices[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		data.vertices[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		data.vertices[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
		data.vertices[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

		// Fill in the front face index data
		data.indices[0] = 0; data.indices[1] = 1; data.indices[2] = 2;
		data.indices[3] = 0; data.indices[4] = 2; data.indices[5] = 3;

		// Fill in the back face index data
		data.indices[6] = 4; data.indices[7] = 5; data.indices[8] = 6;
		data.indices[9] = 4; data.indices[10] = 6; data.indices[11] = 7;

		// Fill in the top face index data
		data.indices[12] = 8; data.indices[13] = 9; data.indices[14] = 10;
		data.indices[15] = 8; data.indices[16] = 10; data.indices[17] = 11;

		// Fill in the bottom face index data
		data.indices[18] = 12; data.indices[19] = 13; data.indices[20] = 14;
		data.indices[21] = 12; data.indices[22] = 14; data.indices[23] = 15;

		// Fill in the left face index data
		data.indices[24] = 16; data.indices[25] = 17; data.indices[26] = 18;
		data.indices[27] = 16; data.indices[28] = 18; data.indices[29] = 19;

		// Fill in the right face index data
		data.indices[30] = 20; data.indices[31] = 21; data.indices[32] = 22;
		data.indices[33] = 20; data.indices[34] = 22; data.indices[35] = 23;

		data.vertices.count = data.vertices.capcity;
		data.indices.count = data.indices.capcity;

		for (uint32 i = 0; i < numSubdivisions; i++) {
			data = Subdivide(&data);
		}

		return ConvertMeshDataIntoModelResource(&data, layout);
	}


}
