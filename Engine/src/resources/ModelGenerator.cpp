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
		Vertex(const Vec3f& pos) { position = pos; }
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

	MeshResource ConvertMeshDataIntoModelResource(MeshData* meshData, VertexLayoutType layout)
	{
		MeshResource result = {};

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

	MeshResource ModelGenerator::CreateQuad(real32 x, real32 y, real32 w, real32 h, real32 depth)
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

	MeshResource ModelGenerator::CreateBox(real32 width, real32 height, real32 depth, uint32 numSubdivisions, VertexLayoutType layout)
	{
		//MeshResource model = {};
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

	MeshResource ModelGenerator::CreateSphere(real32 radius, uint32 sliceCount, uint32 stackCount, VertexLayoutType layout)
	{
		MeshData meshData = {};

		meshData.vertices.Allocate(stackCount * sliceCount + 2, MemoryType::TRANSIENT);
		meshData.indices.Allocate((stackCount * sliceCount + 2) * (stackCount * sliceCount + 2), MemoryType::TRANSIENT);

		// Poles: note that there will be texture coordinate distortion as there is
		// not a unique point on the texture map to assign to the pole when mapping
		// a rectangular texture onto a sphere.
		Vertex topVertex = Vertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		Vertex bottomVertex = Vertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		meshData.vertices.Add(topVertex);

		real32 phiStep = PI / stackCount;
		real32 thetaStep = 2.0f * PI / sliceCount;

		// Compute vertices for each stack ring (do not count the poles as rings).
		for (uint32 i = 1; i <= stackCount - 1; ++i)
		{
			real32 phi = i * phiStep;

			// Vertices of ring.
			for (uint32 j = 0; j <= sliceCount; ++j)
			{
				real32 theta = j * thetaStep;

				Vertex v = {};

				// spherical to cartesian
				v.position.x = radius * sinf(phi) * cosf(theta);
				v.position.y = radius * cosf(phi);
				v.position.z = radius * sinf(phi) * sinf(theta);

				// Partial derivative of P with respect to theta
				v.tanget.x = -radius * sinf(phi) * sinf(theta);
				v.tanget.y = 0.0f;
				v.tanget.z = +radius * sinf(phi) * cosf(theta);

				v.uv.x = theta / (2.0f * PI);
				v.uv.y = phi / PI;

				meshData.vertices.Add(v);
			}
		}

		meshData.vertices.Add(bottomVertex);

		//
		// Compute indices for top stack.  The top stack was written first to the vertex buffer
		// and connects the top pole to the first ring.
		//

		for (uint32 i = 1; i <= sliceCount; ++i)
		{
			meshData.indices.Add(0);
			meshData.indices.Add(i + 1);
			meshData.indices.Add(i);
		}

		//
		// Compute indices for inner stacks (not connected to poles).
		//

		// Offset the indices to the index of the first vertex in the first ring.
		// This is just skipping the top pole vertex.
		uint32 baseIndex = 1;
		uint32 ringVertexCount = sliceCount + 1;
		for (uint32 i = 0; i < stackCount - 2; ++i)
		{
			for (uint32 j = 0; j < sliceCount; ++j)
			{
				meshData.indices.Add(baseIndex + i * ringVertexCount + j);
				meshData.indices.Add(baseIndex + i * ringVertexCount + j + 1);
				meshData.indices.Add(baseIndex + (i + 1) * ringVertexCount + j);

				meshData.indices.Add(baseIndex + (i + 1) * ringVertexCount + j);
				meshData.indices.Add(baseIndex + i * ringVertexCount + j + 1);
				meshData.indices.Add(baseIndex + (i + 1) * ringVertexCount + j + 1);
			}
		}

		//
		// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
		// and connects the bottom pole to the bottom ring.
		//

		// South pole vertex was added last.
		uint32 southPoleIndex = (uint32)meshData.vertices.count - 1;

		// Offset the indices to the index of the first vertex in the last ring.
		baseIndex = southPoleIndex - ringVertexCount;

		for (uint32 i = 0; i < sliceCount; ++i)
		{
			meshData.indices.Add(southPoleIndex);
			meshData.indices.Add(baseIndex + i);
			meshData.indices.Add(baseIndex + i + 1);
		}

		return ConvertMeshDataIntoModelResource(&meshData, layout);
	}

	MeshResource ModelGenerator::CreateGeosphere(real32 radius, uint32 numSubdivisions, VertexLayoutType layout)
	{
		MeshData meshData = {};

		numSubdivisions = Min<uint32>(numSubdivisions, 6);

		// Approximate a sphere by tessellating an icosahedron.
		const real32 X = 0.525731f;
		const real32 Z = 0.850651f;

		meshData.vertices.Allocate(12, MemoryType::TRANSIENT);
		meshData.indices.Allocate(60, MemoryType::TRANSIENT);

		meshData.vertices.Add(Vec3f(-X, 0.0f, Z));	meshData.vertices.Add(Vec3f(X, 0.0f, Z));
		meshData.vertices.Add(Vec3f(-X, 0.0f, -Z)); meshData.vertices.Add(Vec3f(X, 0.0f, -Z));
		meshData.vertices.Add(Vec3f(0.0f, Z, X));   meshData.vertices.Add(Vec3f(0.0f, Z, -X));
		meshData.vertices.Add(Vec3f(0.0f, -Z, X));  meshData.vertices.Add(Vec3f(0.0f, -Z, -X));
		meshData.vertices.Add(Vec3f(Z, X, 0.0f));   meshData.vertices.Add(Vec3f(-Z, X, 0.0f));
		meshData.vertices.Add(Vec3f(Z, -X, 0.0f));  meshData.vertices.Add(Vec3f(-Z, -X, 0.0f));

		uint32 k[60] =
		{
			1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
			1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
			3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
			10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
		};

		for (uint32 i = 0; i < ArrayCount(k); i++) { meshData.indices.Add(k[i]); }
		for (uint32 i = 0; i < numSubdivisions; ++i) { meshData = Subdivide(&meshData); }

		// Project vertices onto sphere and scale.
		for (uint32 i = 0; i < meshData.vertices.count; ++i)
		{
			// Project onto unit sphere.
			Vec3f n = Normalize(meshData.vertices[i].position);

			// Project onto sphere.
			Vec3f p = radius * n;

			meshData.vertices[i].position = p;
			meshData.vertices[i].normal = n;

			// Derive texture coordinates from spherical coordinates.
			real32 theta = atan2f(meshData.vertices[i].position.z, meshData.vertices[i].position.x);

			// Put in [0, 2pi].
			if (theta < 0.0f) { theta += 2.0f * PI; }

			real32 phi = acosf(meshData.vertices[i].position.y / radius);

			meshData.vertices[i].uv.x = theta / (2.0f * PI);
			meshData.vertices[i].uv.y = phi / (PI);

			// Partial derivative of P with respect to theta
			meshData.vertices[i].tanget.x = -radius * sinf(phi) * sinf(theta);
			meshData.vertices[i].tanget.y = 0.0f;
			meshData.vertices[i].tanget.z = +radius * sinf(phi) * cosf(theta);

			meshData.vertices[i].tanget = Normalize(meshData.vertices[i].tanget);

		}

		return ConvertMeshDataIntoModelResource(&meshData, layout);
	}

	static void BuildCylinderTopCap(real32  bottomRadius, real32  topRadius, real32 height,
		uint32 sliceCount, uint32 stackCount, MeshData* meshData)
	{
		uint32 baseIndex = (uint32)meshData->vertices.count;

		real32 y = 0.5f * height;
		real32 dTheta = 2.0f * PI / sliceCount;

		// Duplicate cap ring vertices because the texture coordinates and normals differ.
		for (uint32 i = 0; i <= sliceCount; ++i)
		{
			real32 x = topRadius * cosf(i * dTheta);
			real32 z = topRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			real32 u = x / height + 0.5f;
			real32 v = z / height + 0.5f;

			meshData->vertices.Add(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
		}

		// Cap center vertex.
		meshData->vertices.Add(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

		// Index of center vertex.
		uint32 centerIndex = (uint32)meshData->vertices.count - 1;

		for (uint32 i = 0; i < sliceCount; ++i)
		{
			meshData->indices.Add(centerIndex);
			meshData->indices.Add(baseIndex + i + 1);
			meshData->indices.Add(baseIndex + i);
		}
	}

	static void BuildCylinderBottomCap(real32 bottomRadius, real32 topRadius, real32 height,
		uint32 sliceCount, uint32 stackCount, MeshData* meshData)
	{
		// 
		// Build bottom cap.
		//

		uint32 baseIndex = (uint32)meshData->vertices.count;
		real32 y = -0.5f * height;

		// vertices of ring
		real32 dTheta = 2.0f * PI / sliceCount;
		for (uint32 i = 0; i <= sliceCount; ++i)
		{
			real32 x = bottomRadius * cosf(i * dTheta);
			real32 z = bottomRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			real32 u = x / height + 0.5f;
			real32 v = z / height + 0.5f;

			meshData->vertices.Add(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
		}

		// Cap center vertex.
		meshData->vertices.Add(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

		// Cache the index of center vertex.
		uint32 centerIndex = (uint32)meshData->vertices.count - 1;

		for (uint32 i = 0; i < sliceCount; ++i)
		{
			meshData->indices.Add(centerIndex);
			meshData->indices.Add(baseIndex + i);
			meshData->indices.Add(baseIndex + i + 1);
		}
	}

	MeshResource ModelGenerator::CreateCylinder(real32 bottomRadius, real32 topRadius, real32 height,
		uint32 sliceCount, uint32 stackCount, VertexLayoutType layout)
	{
		MeshData meshData = {};

		real32 stackHeight = height / stackCount;
		real32 radiusStep = (topRadius - bottomRadius) / stackCount;
		uint32 ringCount = stackCount + 1;

		meshData.vertices.Allocate(2048, MemoryType::TRANSIENT);
		meshData.indices.Allocate(4096, MemoryType::TRANSIENT);

		for (uint32 i = 0; i < ringCount; ++i)
		{
			real32 y = -0.5f * height + i * stackHeight;
			real32 r = bottomRadius + i * radiusStep;

			real32 dTheta = 2.0f * PI / sliceCount;
			for (uint32 j = 0; j <= sliceCount; ++j)
			{
				Vertex vertex = {};

				real32 c = Cos(j * dTheta);
				real32 s = Sin(j * dTheta);

				vertex.position = Vec3f(r * c, y, r * s);
				vertex.uv.x = (real32)j / sliceCount;
				vertex.uv.y = 1.0f - (real32)i / stackCount;

				vertex.tanget = Vec3f(-s, 0.0f, c);

				real32 dr = bottomRadius - topRadius;
				Vec3f bitangent = Vec3f(dr * c, -height, dr * s);

				vertex.normal = Normalize(Cross(vertex.tanget, bitangent));

				meshData.vertices.Add(vertex);
			}
		}

		// @NOTE: Add one because we duplicate the first and last vertex per ring
		// @NOTE: since the texture coordinates are different.
		uint32 ringVertexCount = sliceCount + 1;

		for (uint32 i = 0; i < stackCount; ++i)
		{
			for (uint32 j = 0; j < sliceCount; ++j)
			{
				meshData.indices.Add(i * ringVertexCount + j);
				meshData.indices.Add((i + 1) * ringVertexCount + j);
				meshData.indices.Add((i + 1) * ringVertexCount + j + 1);
				meshData.indices.Add(i * ringVertexCount + j);
				meshData.indices.Add((i + 1) * ringVertexCount + j + 1);
				meshData.indices.Add(i * ringVertexCount + j + 1);
			}
		}

		BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, &meshData);
		BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, &meshData);

		return ConvertMeshDataIntoModelResource(&meshData, layout);
	}
}
