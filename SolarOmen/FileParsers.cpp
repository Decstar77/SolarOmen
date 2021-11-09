#include "FileParsers.h"
#if 0
#include "SolarOmen.h"
#include "renderer/SolarRenderer.h"
#include <unordered_map>
#include <fstream>
namespace cm
{
	int32 DEBUGCreateShaderFromBinary(RenderState* rs, AssetState* as, const VertexShaderLayout& layout, CString vertex_file, CString pixel_file)
	{
		ShaderInstance shader = {};

		PlatformFile vertex_code = DEBUGLoadEntireFile(vertex_file, false);
		PlatformFile pixel_code = DEBUGLoadEntireFile(pixel_file, false);

		Assert(vertex_code.data, "Could not find vertex code");
		Assert(pixel_code.data, "Could not find pixel code");

		switch (layout)
		{
		case VertexShaderLayout::P:
		{
			D3D11_INPUT_ELEMENT_DESC pos_desc = {};
			pos_desc.SemanticName = "Position";
			pos_desc.SemanticIndex = 0;
			pos_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pos_desc.InputSlot = 0;
			pos_desc.AlignedByteOffset = 0;
			pos_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pos_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC layouts[] = { pos_desc };

			DXCHECK(rs->device->CreateInputLayout(layouts, 1, vertex_code.data,
				vertex_code.size_bytes, &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(vertex_code.data,
				vertex_code.size_bytes, nullptr, &shader.vs_shader));

			DXCHECK(rs->device->CreatePixelShader(pixel_code.data,
				pixel_code.size_bytes, nullptr, &shader.ps_shader));

		} break;
		case VertexShaderLayout::P_PAD:
		{
			D3D11_INPUT_ELEMENT_DESC pos_desc = {};
			pos_desc.SemanticName = "Position";
			pos_desc.SemanticIndex = 0;
			pos_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			pos_desc.InputSlot = 0;
			pos_desc.AlignedByteOffset = 0;
			pos_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pos_desc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC layouts[] = { pos_desc };

			DXCHECK(rs->device->CreateInputLayout(layouts, 1, vertex_code.data,
				vertex_code.size_bytes, &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(vertex_code.data,
				vertex_code.size_bytes, nullptr, &shader.vs_shader));

			DXCHECK(rs->device->CreatePixelShader(pixel_code.data,
				pixel_code.size_bytes, nullptr, &shader.ps_shader));
		} break;
		case VertexShaderLayout::PNT:
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

			DXCHECK(rs->device->CreateInputLayout(layouts, 3, vertex_code.data,
				vertex_code.size_bytes, &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(vertex_code.data,
				vertex_code.size_bytes, nullptr, &shader.vs_shader));

			DXCHECK(rs->device->CreatePixelShader(pixel_code.data,
				pixel_code.size_bytes, nullptr, &shader.ps_shader));
		} break;
		case VertexShaderLayout::PNTM:
		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				// Data from the vertex buffer
				{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{ "TexCord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

				// Data from the instance buffer
				{ "Model", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{ "Model", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{ "Model", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{ "Model", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			};


			DXCHECK(rs->device->CreateInputLayout(layout, 7, vertex_code.data,
				vertex_code.size_bytes, &shader.layout));

			DXCHECK(rs->device->CreateVertexShader(vertex_code.data,
				vertex_code.size_bytes, nullptr, &shader.vs_shader));

			DXCHECK(rs->device->CreatePixelShader(pixel_code.data,
				pixel_code.size_bytes, nullptr, &shader.ps_shader));
		}break;
		}

		int32 index = as->shaderCount;
		as->shaderCount++;

		rs->shaders[index] = shader;
		ShaderData* shaderData = &as->shadersData[index];

		shaderData->vertexLayout = layout;
		shaderData->vertexPath = vertex_file;
		shaderData->pixelPath = pixel_file;
		shaderData->name = Util::StripFilePathAndExtentions(vertex_file);

		DEBUGFreeFile(&vertex_code);
		DEBUGFreeFile(&pixel_code);



		return index;
	}

	struct TemporayMeshBuffer
	{
		std::vector<Vec3f> positions;
		std::vector<Vec3f> normals;
		std::vector<Vec2f> uvs;

		std::vector<real32> vertices;
		std::vector<uint32> indices;
		std::unordered_map<std::string, uint32> processed;

		uint32 indices_counter;
	};

	struct TemporyVoxelBuffer
	{
		std::vector<Vec3f> points;
		std::vector<AABB> boxes;
		std::vector<Voxel> voxels;

		Vec3f offset;
		int32 width;
		int32 height;
		int32 depth;

		inline Voxel* GetRight(Voxel* voxel)
		{
			int32 x = voxel->xIndex + 1;
			int32 y = voxel->yIndex;
			int32 z = voxel->zIndex;
			if (InBounds(x, y, z))
				return &voxels[GetIndex(x, y, z)];
			return nullptr;
		}

		inline Voxel* GetLeft(Voxel* voxel)
		{
			int32 x = voxel->xIndex - 1;
			int32 y = voxel->yIndex;
			int32 z = voxel->zIndex;
			if (InBounds(x, y, z))
				return &voxels[GetIndex(x, y, z)];
			return nullptr;
		}

		inline Voxel* GetFront(Voxel* voxel)
		{
			int32 x = voxel->xIndex;
			int32 y = voxel->yIndex;
			int32 z = voxel->zIndex + 1;
			if (InBounds(x, y, z))
				return &voxels[GetIndex(x, y, z)];
			return nullptr;
		}

		inline Voxel* GetBack(Voxel* voxel)
		{
			int32 x = voxel->xIndex;
			int32 y = voxel->yIndex;
			int32 z = voxel->zIndex - 1;
			if (InBounds(x, y, z))
				return &voxels[GetIndex(x, y, z)];
			return nullptr;
		}

		inline Voxel* GetUp(Voxel* voxel)
		{
			int32 x = voxel->xIndex;
			int32 y = voxel->yIndex + 1;
			int32 z = voxel->zIndex;
			if (InBounds(x, y, z))
				return &voxels[GetIndex(x, y, z)];
			return nullptr;
		}

		inline Voxel* GetDown(Voxel* voxel)
		{
			int32 x = voxel->xIndex;
			int32 y = voxel->yIndex - 1;
			int32 z = voxel->zIndex;
			if (InBounds(x, y, z))
				return &voxels[GetIndex(x, y, z)];
			return nullptr;
		}

		inline int32 GetIndex(int32 x, int32 y, int32 z)
		{
			Assert(InBounds(x, y, z), "Voxel index not in bounds");

			return (z * width * height) + (y * width) + x;
		}

		inline Vec3i GetPosIndex(int32 index)
		{
			int32 z = index / (width * height);
			index -= (z * width * height);
			int32 y = index / width;
			int32 x = index % width;

			return Vec3i(x, y, z);
		}

		inline bool InBounds(int32 x, int32 y, int32 z)
		{
			return x >= 0 && x < width&&
				y >= 0 && y < height&&
				z >= 0 && z < depth;
		}
	};

	static void ProcessVertex(TemporayMeshBuffer* mesh, const CString& key, const std::vector<CString>& vertex_data)
	{
		if (mesh->processed.find(key.GetCStr()) == mesh->processed.end())
		{
			int32 pi = vertex_data.at(0).ToInt32() - 1;
			int32 ni = vertex_data.at(2).ToInt32() - 1;
			int32 ti = vertex_data.at(1).ToInt32() - 1;

			Assert(pi < mesh->positions.size(), "a");

			Vec3f position = mesh->positions.at(pi);
			Vec3f normal = mesh->normals.at(ni);
			Vec2f texture = mesh->uvs.at(ti);

			mesh->vertices.push_back(position.x);
			mesh->vertices.push_back(position.y);
			mesh->vertices.push_back(position.z);

			mesh->vertices.push_back(normal.x);
			mesh->vertices.push_back(normal.y);
			mesh->vertices.push_back(normal.z);

			mesh->vertices.push_back(texture.x);
			mesh->vertices.push_back(texture.y);

			mesh->processed[key.GetCStr()] = mesh->indices_counter;
			mesh->indices.push_back(mesh->indices_counter);

			mesh->indices_counter++;
		}
		else
		{
			uint32 index = mesh->processed[key.GetCStr()];
			mesh->indices.push_back(index);
		}
	}

	static void FillMeshData(AssetState* as, MemoryArena* mem, TemporayMeshBuffer& mesh_data, CString name)
	{
		int32 index = as->meshCount;
		MeshData* data = &as->meshesData[index];
		data->name = name;

		int32 position_size_bytes = (int32)mesh_data.positions.size() * sizeof(Vec3f);
		data->position_count = (int32)mesh_data.positions.size();
		data->positions = (Vec3f*)mem->PushSize_(position_size_bytes);
		memcpy(data->positions, mesh_data.positions.data(), position_size_bytes);

		int32 normals_size_bytes = (int32)mesh_data.normals.size() * sizeof(Vec3f);
		data->normals_count = (int32)mesh_data.normals.size();
		data->normals = (Vec3f*)mem->PushSize_(normals_size_bytes);
		memcpy(data->normals, mesh_data.normals.data(), normals_size_bytes);

		int32 uvs_size_bytes = (int32)mesh_data.uvs.size() * sizeof(Vec2f);
		data->uvs_count = (int32)mesh_data.uvs.size();
		data->uvs = (Vec2f*)mem->PushSize_(uvs_size_bytes);
		memcpy(data->uvs, mesh_data.uvs.data(), uvs_size_bytes);

		int32 vertex_size_bytes = (int32)mesh_data.vertices.size() * sizeof(real32);
		data->packed_count = (int32)mesh_data.vertices.size();
		data->packed_vertices = (real32*)mem->PushSize_(vertex_size_bytes);
		memcpy(data->packed_vertices, mesh_data.vertices.data(), vertex_size_bytes);

		int32 indices_size_bytes = (int32)mesh_data.indices.size() * sizeof(uint32);
		data->indices_count = (int32)mesh_data.indices.size();
		data->indices = (uint32*)mem->PushSize_(indices_size_bytes);
		memcpy(data->indices, mesh_data.indices.data(), indices_size_bytes);

		data->packed_stride = 3 + 3 + 2;

		Vec3f min = Vec3f(100000.0f);
		Vec3f max = Vec3f(-100000.0f);

		for (const Vec3f& vert : mesh_data.positions)
		{
			min.x = Min(min.x, vert.x);
			min.y = Min(min.y, vert.y);
			min.z = Min(min.z, vert.z);

			max.x = Max(max.x, vert.x);
			max.y = Max(max.y, vert.y);
			max.z = Max(max.z, vert.z);
		}

		data->bounding_box = CreateAABBFromMinMax(min, max);
	}

	static void FillMeshInstance(RenderState* rs, AssetState* as, TemporayMeshBuffer& mesh_data)
	{
		MeshInstance result = CreateMeshInstance(rs, mesh_data.vertices.data(),
			(int32)mesh_data.vertices.size(), mesh_data.indices.data(), (int32)mesh_data.indices.size());
		int32 index = as->meshCount;
		rs->meshes[index] = result;
	}

	static void FillMeshVoxelData(RenderState* rs, AssetState* as, MemoryArena* mem, TemporyVoxelBuffer& vox)
	{
		int32 index = as->meshCount;
		MeshData* data = &as->meshesData[index];

		int32 sizeBytes = (int32)vox.boxes.size() * sizeof(AABB);
		data->shadowBoxCount = (int32)vox.boxes.size();
		data->shadowBoxes = (AABB*)mem->PushSize_(sizeBytes);
		memcpy(data->shadowBoxes, vox.boxes.data(), sizeBytes);
	}

	static TemporayMeshBuffer DEBUGLoadOBJMesh(PlatformFile objFile)
	{
		LargeString file = {};
		file.Wrap((char*)objFile.data, objFile.size_bytes); // @STRING WARN: file_size is int32

		TemporayMeshBuffer mesh_data = { };
		mesh_data.indices_counter = 0;

		CString line;
		while ((line = file.GetLine()).GetLength() > 0)
		{
			std::vector<CString> currentLine = line.Split(' ');

			if (line.StartsWith("v "))
			{
				Vec3f pos;
				pos.x = (currentLine.at(1)).ToReal32();
				pos.y = (currentLine.at(2)).ToReal32();
				pos.z = (currentLine.at(3)).ToReal32();

				mesh_data.positions.push_back(pos);
			}
			else if (line.StartsWith("vt "))
			{
				Vec2f UV;
				UV.x = (currentLine.at(1)).ToReal32();
				UV.y = (currentLine.at(2)).ToReal32();

				mesh_data.uvs.push_back(UV);
			}
			else if (line.StartsWith("vn "))
			{
				Vec3f n;
				n.x = (currentLine.at(1)).ToReal32();
				n.y = (currentLine.at(2)).ToReal32();
				n.z = (currentLine.at(3)).ToReal32();

				mesh_data.normals.push_back(n);
			}
			else if (line.StartsWith("f "))
			{
				break;
			}
		}

		Mat3f scale = ScaleCardinal(Mat3f(1.0f), Vec3f(MODEL_IMPORT_SCALE));

		for (int32 i = 0; i < mesh_data.positions.size(); i++)
		{
			mesh_data.positions.at(i) = mesh_data.positions.at(i) * scale;
		}

		do
		{
			if (line.StartsWith("f "))
			{
				std::vector<CString> currentLine = line.Split(' ');
				std::vector<CString> keys = currentLine;
				if (keys.size() >= 4)
				{
					std::vector<CString> v1 = currentLine.at(1).Split('/');
					std::vector<CString> v2 = currentLine.at(2).Split('/');
					std::vector<CString> v3 = currentLine.at(3).Split('/');

					ProcessVertex(&mesh_data, keys.at(1), v1);
					ProcessVertex(&mesh_data, keys.at(2), v2);
					ProcessVertex(&mesh_data, keys.at(3), v3);
				}
			}

		} while ((line = file.GetLine()).GetLength() > 0);

		return mesh_data;
	}

	static TemporayMeshBuffer DEBUGLoadOBJMesh(CString path)
	{
		PlatformFile mesh_file = DEBUGLoadEntireFile(path, false);
		Assert(mesh_file.data, "OBJ Mesh file not found");

		TemporayMeshBuffer result = DEBUGLoadOBJMesh(mesh_file);

		DEBUGFreeFile(&mesh_file);

		return result;
	}


	int32 DEBUGCreateOBJMesh(RenderState* rs, AssetState* as, MemoryArena* mem, PlatformFile objFile)
	{
		TemporayMeshBuffer mesh_data = DEBUGLoadOBJMesh(objFile);

		CString name = Util::StripFilePath(objFile.path);

		FillMeshData(as, mem, mesh_data, name);
		FillMeshInstance(rs, as, mesh_data);
		int32 index = as->meshCount;
		as->meshCount++;

		return index;
	}

	int32 DEBUGCreateOBJMesh(RenderState* rs, AssetState* as, MemoryArena* mem, CString path)
	{
		PlatformFile mesh_file = DEBUGLoadEntireFile(path, false);
		Assert(mesh_file.data, "OBJ Mesh file not found");

		CString name = Util::StripFilePath(path);

		int32 index = DEBUGCreateOBJMesh(rs, as, mem, mesh_file);

		DEBUGFreeFile(&mesh_file);

		return index;
	}

	std::vector<Voxel*> SweepVoxXAxis(TemporyVoxelBuffer* vox, Voxel* voxel, int32 boxIndex)
	{
		std::vector<Voxel*> voxels;
		if (voxel->boxIndex == -1)
		{
			voxel->boxIndex = boxIndex;

			voxels.push_back(voxel);
			{
				Voxel* right = vox->GetRight(voxel);

				while (right && right->boxIndex == -1 && right->IsValid())
				{
					right->boxIndex = boxIndex;
					voxels.push_back(right);

					right = vox->GetRight(right);
				}
			}

			{
				Voxel* left = vox->GetLeft(voxel);

				while (left && left->boxIndex == -1 && left->IsValid())
				{
					left->boxIndex = boxIndex;
					voxels.push_back(left);

					left = vox->GetLeft(left);
				}
			}
		}

		return voxels;
	}

	std::vector<Voxel*> MergeVoxBackward(TemporyVoxelBuffer* vox, std::vector<Voxel*>* group, const std::vector<Voxel*>& fringe, int32 boxIndex)
	{
		std::vector<Voxel*> frontier;

		for (int32 i = 0; i < fringe.size(); i++)
		{
			Voxel* v = vox->GetBack(fringe.at(i));
			if (v && v->boxIndex == -1 && v->IsValid())
			{
				frontier.push_back(v);
			}
		}

		if (frontier.size() == fringe.size())
		{
			for (int32 i = 0; i < frontier.size(); i++)
			{
				Voxel* v = frontier.at(i);
				v->boxIndex = boxIndex;
				group->push_back(v);
			}
		}
		else
		{
			frontier.clear();
		}

		return frontier;
	}

	std::vector<Voxel*> MergeVoxForward(TemporyVoxelBuffer* vox, std::vector<Voxel*>* group, const std::vector<Voxel*>& fringe, int32 boxIndex)
	{
		std::vector<Voxel*> frontier;

		for (int32 i = 0; i < fringe.size(); i++)
		{
			Voxel* v = vox->GetFront(fringe.at(i));
			if (v && v->boxIndex == -1 && v->IsValid())
			{
				frontier.push_back(v);
			}
		}

		if (frontier.size() == fringe.size())
		{
			for (int32 i = 0; i < frontier.size(); i++)
			{
				Voxel* v = frontier.at(i);
				v->boxIndex = boxIndex;
				group->push_back(v);
			}
		}
		else
		{
			frontier.clear();
		}

		return frontier;
	}

	std::vector<Voxel*> MergeVoxUpward(TemporyVoxelBuffer* vox, std::vector<Voxel*>* group,
		const std::vector<Voxel*>& fringe, int32 boxIndex)
	{
		std::vector<Voxel*> frontier;

		for (int32 i = 0; i < fringe.size(); i++)
		{
			Voxel* v = vox->GetUp(fringe.at(i));
			if (v && v->boxIndex == -1 && v->IsValid())
			{
				frontier.push_back(v);
			}
		}

		if (frontier.size() == fringe.size())
		{
			for (int32 i = 0; i < frontier.size(); i++)
			{
				Voxel* v = frontier.at(i);
				v->boxIndex = boxIndex;
				group->push_back(v);
			}
		}
		else
		{
			frontier.clear();
		}

		return frontier;
	}

	std::vector<Voxel*> MergeVoxDownward(TemporyVoxelBuffer* vox, std::vector<Voxel*>* group,
		const std::vector<Voxel*>& fringe, int32 boxIndex)
	{
		std::vector<Voxel*> frontier;

		for (int32 i = 0; i < fringe.size(); i++)
		{
			Voxel* v = vox->GetDown(fringe.at(i));
			if (v && v->boxIndex == -1 && v->IsValid())
			{
				frontier.push_back(v);
			}
		}

		if (frontier.size() == fringe.size())
		{
			for (int32 i = 0; i < frontier.size(); i++)
			{
				Voxel* v = frontier.at(i);
				v->boxIndex = boxIndex;
				group->push_back(v);
			}
		}
		else
		{
			frontier.clear();
		}

		return frontier;
	}

	void OptimizeVoxMesh(TemporyVoxelBuffer* vox)
	{
		for (int32 i = 0; i < vox->voxels.size(); i++)
		{
			Voxel* voxel = &vox->voxels[i];
			voxel->boxIndex = -1;
			voxel->xIndex = -1;
			voxel->yIndex = -1;
			voxel->zIndex = -1;
		}

		for (int32 i = 0; i < vox->points.size(); i++)
		{
			Vec3f p = vox->points[i] + vox->offset;

			int32 x = (int32)p.x;
			int32 y = (int32)p.y;
			int32 z = (int32)p.z;

			int32 index = vox->GetIndex(x, y, z);

			Voxel* voxel = &vox->voxels[index];
			voxel->xIndex = x;
			voxel->yIndex = y;
			voxel->zIndex = z;
		}

		int32 boxIndex = -1;
		for (int32 i = 0; i < vox->points.size(); i++)
		{
			Vec3f p = vox->points[i] + vox->offset;
			int32 x = (int32)p.x;
			int32 y = (int32)p.y;
			int32 z = (int32)p.z;

			int32 index = vox->GetIndex(x, y, z);

			Voxel* voxel = &vox->voxels[index];
			if (voxel->boxIndex == -1)
			{
				boxIndex++;

				std::vector<Voxel*> group = SweepVoxXAxis(vox, voxel, boxIndex);
				std::vector<Voxel*> fringe = group;

				{
					std::vector<Voxel*> backward = fringe;
					do
					{
						backward = MergeVoxBackward(vox, &group, backward, boxIndex);
					} while (backward.size() != 0);
				}
				{
					std::vector<Voxel*> forward = fringe;
					do
					{
						forward = MergeVoxForward(vox, &group, forward, boxIndex);
					} while (forward.size() != 0);
				}

				fringe = group;

				{
					std::vector<Voxel*> upward = fringe;
					do
					{
						upward = MergeVoxUpward(vox, &group, upward, boxIndex);
					} while (upward.size() != 0);
				}
				{
					std::vector<Voxel*> downward = fringe;
					do
					{
						downward = MergeVoxUpward(vox, &group, downward, boxIndex);
					} while (downward.size() != 0);
				}

			}

		}

		for (int32 i = 0; i < boxIndex + 1; i++)
		{
			vox->boxes.push_back(CreateAABBEmpty());
		}

		for (int32 i = 0; i < vox->points.size(); i++)
		{
			Vec3f p = vox->points[i] + vox->offset;
			int32 x = (int32)p.x;
			int32 y = (int32)p.y;
			int32 z = (int32)p.z;

			int32 index = vox->GetIndex(x, y, z);

			Voxel* voxel = &vox->voxels[index];

			Assert(voxel->boxIndex != -1, "A voxel without a box!!");
			vox->boxes[voxel->boxIndex] = ExtendAABB(vox->boxes[voxel->boxIndex], CreateAABBFromCenterRadius(p, Vec3f(0.5)));
		}

		Mat3f scale = ScaleCardinal(Mat3f(1.0f), Vec3f(VOXEL_IMPORT_SCALE));

		for (int32 i = 0; i < vox->boxes.size(); i++)
		{
			vox->boxes[i].min -= vox->offset - Vec3f(0.5f, 0.5, -0.5);
			vox->boxes[i].max -= vox->offset - Vec3f(0.5f, 0.5, -0.5);
		}

		for (int32 i = 0; i < vox->boxes.size(); i++)
		{
			vox->boxes[i].min = vox->boxes[i].min * scale;
			vox->boxes[i].max = vox->boxes[i].max * scale;
		}

		LOG("Total point count: " << vox->points.size());
		LOG("Total box count: " << vox->boxes.size());
		LOG("Reduction: " << (100.0f - ((real32)vox->boxes.size() / (real32)(vox->points.size()) * 100.0f)) << "%");
	}


	static TemporyVoxelBuffer DEBUGLoadVoxMesh(PlatformFile voxFile)
	{
		TemporyVoxelBuffer voxelBuffer = {};

		LargeString file;
		file.Wrap((char*)voxFile.data, voxFile.size_bytes);

		CString line;
		while ((line = file.GetLine()).GetLength() > 0)
		{
			if (line.StartsWith("end_header"))
			{
				break;
			}
		}

		Vec3f min = Vec3f(REAL_MAX);
		Vec3f max = Vec3f(REAL_MIN);

		while ((line = file.GetLine()).GetLength() > 0)
		{
			std::vector<CString> values = line.Split(' ');
			if (values.size() < 6)
				break;

			real32 x = values.at(0).ToReal32();
			real32 z = -values.at(1).ToReal32();
			real32 y = values.at(2).ToReal32();

			Vec3f p = Vec3f(x, y, z);

			voxelBuffer.points.push_back(p);

			min = Min(min, p);
			max = Max(max, p);
		}

		max += Abs(min);

		voxelBuffer.offset = Abs(min);
		voxelBuffer.width = (int32)max.x + 1;
		voxelBuffer.height = (int32)max.y + 1;
		voxelBuffer.depth = (int32)max.z + 1;
		voxelBuffer.voxels.resize(voxelBuffer.width * voxelBuffer.height * voxelBuffer.depth);

		return voxelBuffer;
	}

	static TemporyVoxelBuffer DEBUGLoadVoxMesh(CString path)
	{
		PlatformFile meshFile = DEBUGLoadEntireFile(path, false);
		Assert(meshFile.data, "Voxel Mesh file not found");

		TemporyVoxelBuffer result = DEBUGLoadVoxMesh(meshFile);

		DEBUGFreeFile(&meshFile);

		return result;
	}


	int32 DEBUGCreateMesh(RenderState* rs, AssetState* as, MemoryArena* mem, PlatformFile objFile, PlatformFile voxFile)
	{
		TemporayMeshBuffer meshBuffer = DEBUGLoadOBJMesh(objFile);
		TemporyVoxelBuffer voxBuffer = DEBUGLoadVoxMesh(voxFile);

		OptimizeVoxMesh(&voxBuffer);

		CString meshName = Util::StripFilePath(objFile.path);

		FillMeshData(as, mem, meshBuffer, meshName);
		FillMeshVoxelData(rs, as, mem, voxBuffer);
		FillMeshInstance(rs, as, meshBuffer);

		int32 index = as->meshCount;
		as->meshCount++;

		return index;
	}

	int32 DEBUGCreateMesh(RenderState* rs, AssetState* as, MemoryArena* mem, CString objPath, CString voxPath)
	{
		TemporayMeshBuffer meshBuffer = DEBUGLoadOBJMesh(objPath);
		TemporyVoxelBuffer voxBuffer = DEBUGLoadVoxMesh(voxPath);

		OptimizeVoxMesh(&voxBuffer);

		CString meshName = Util::StripFilePath(objPath);

		FillMeshData(as, mem, meshBuffer, meshName);
		FillMeshVoxelData(rs, as, mem, voxBuffer);
		FillMeshInstance(rs, as, meshBuffer);

		int32 index = as->meshCount;
		as->meshCount++;

		return index;
	}

	void DEBUGCreateAllMeshes(RenderState* rs, AssetState* as, MemoryArena* mem, CString folderPath)
	{
		PlatformFolder objFolder = DEBUGLoadEnitreFolder(folderPath, "*.obj", false);
		PlatformFolder voxFolder = DEBUGLoadEnitreFolder(folderPath, "*.ply", false);

		int32 voxFileIndex = 0;
		for (int32 objFileIndex = 0;
			objFileIndex < objFolder.files.size();
			objFileIndex++)
		{
			PlatformFile objFile = objFolder.files[objFileIndex];
			CString objName = Util::StripFilePathAndExtentions(objFile.path);

			PlatformFile voxFile = {};
			CString voxName = {};

			if (voxFileIndex < voxFolder.files.size())
			{
				voxFile = voxFolder.files[voxFileIndex];
				voxName = Util::StripFilePathAndExtentions(voxFile.path);
			}

			if (objName == voxName)
			{
				int32 index = DEBUGCreateMesh(rs, as, mem, objFile, voxFile);
				voxFileIndex++;
			}
			else
			{
				LOG("Warning " << objName.GetCStr() << " doesn't have a corrasponding voxel mesh");
				DEBUGCreateOBJMesh(rs, as, mem, objFile);
			}
		}

		DEBUGFreeFolder(&objFolder);
		DEBUGFreeFolder(&voxFolder);
	}

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/std_image/std_image.h"

	int32 DEBUGCreateTexture(RenderState* rs, AssetState* as, CString path, bool flip, bool mips)
	{
		PlatformFile file = DEBUGLoadEntireFile(path, false);

		int32 width = -1;
		int32 height = -1;
		int32 channels = -1;
		stbi_set_flip_vertically_on_load(flip);

		uint8* pixels = stbi_load_from_memory((stbi_uc*)file.data, (int32)file.size_bytes,
			&width, &height, &channels, 4);

		Assert(pixels, CString("Cannot find image: ").Add(path.GetCStr()).GetCStr());

		// @NOTE: We asked stbi to pad it with an aplha value wheather or not it is there.
		// @NOTE: The channel value is the TRUE amount of channel without pad/req comps count.
		// @NOTE: Thus we set it to 4
		channels = 4;

		TextureCreateInfo cinfo = {};
		cinfo.channels = channels;
		cinfo.width = width;
		cinfo.height = height;
		cinfo.pixels = pixels;
		cinfo.format = TextureFormat::R8G8B8A8_UNORM;
		cinfo.usage[0] = TextureUsage::SHADER_RESOURCE;

		TextureInstance texture = CreateTextureInstance2D(rs, cinfo);

		DEBUGFreeFile(&file);

		TextureData textureData = {};
		textureData.info = cinfo;
		textureData.name = Util::StripFilePathAndExtentions(path);

		int32 index = as->textureCount;
		rs->textures[index] = texture;
		as->texturesData[index] = {};
		as->texturesData[index] = textureData;
		as->textureCount++;

		return index;
	}

	CubeMapInstance DEBUGCreateCubeMap(RenderState* rs, CString path)
	{
		stbi_set_flip_vertically_on_load(false);

		int32 width = -1;
		int32 height = -1;
		int32 channels = -1;

		uint8* sides[6] = {};

		for (int32 i = 0; i < 6; i++)
		{
			PlatformFile file = DEBUGLoadEntireFile(CString(path).Add("/").Add(i).Add(".png"), false);

			sides[i] = stbi_load_from_memory((stbi_uc*)file.data, (int32)file.size_bytes,
				&width, &height, &channels, 4);

			Assert(sides[i] != nullptr, "Cube map face not found");

			DEBUGFreeFile(&file);
		}

		// @NOTE: We asked stbi to pad it with an aplha value wheather or not it is there.
		// @NOTE: The channel value is the TRUE amount of channel without pad/req comps count.
		// @NOTE: Thus we set it to 4

		channels = 4;
		CubeMapInstance cube = CreateCubeMapInstance(rs, sides, width, height, channels);

		return cube;
	}



	inline static CString ToString(const OBB& obb)
	{
		CString ss = {};

		for (int32 i = 0; i < 16; i++)
		{
			ss.Add(obb.mat.ptr[i]).Add(" ");
		}

		ss.Add("Extents=").Add(ToString(obb.extents).GetCStr());

		return ss;
	}

	inline static CString ToString(const AABB& aabb)
	{
		CString ss = {};

		ss.Add(ToString(aabb.min)).Add(" ");
		ss.Add(ToString(aabb.max)).Add(" ");

		return ss;
	}

	inline static CString SaveRenderComponent(GameState* gs, AssetState* as, RenderState* rs, Entity* entity)
	{
		CString ss = {};
		ss.Add("\tRenderComponent:\n");

		if (entity->render.mesh)
		{
			MeshData* meshData = LookUpMeshData(as, entity->render.mesh);
			ss.Add("\t\tMesh=").Add(meshData->name).Add("\n");
		}
		else
		{
			ss.Add("\t\tMesh=__NONE__\n");
		}

		if (entity->render.texture)
		{
			TextureData* textureData = LookUpTextureData(as, entity->render.texture);
			ss.Add("\t\tText=").Add(textureData->name).Add("\n");
		}
		else
		{
			ss.Add("\t\tText=__NONE__\n");
		}

		if (entity->render.shader)
		{
			ShaderData* shaderData = LookUpShaderData(as, entity->render.shader);
			ss.Add("\t\tShdr=").Add(shaderData->name).Add("\n");
		}
		else
		{
			ss.Add("\t\tShdr=__NONE__\n");
		}

		ss.Add("\t\tFlags=").Add((int32)entity->render.flags).Add("\n");

		return ss;
	}

	inline static CString SaveLightComponent(Entity* entity)
	{
		CString ss = {};
		ss.Add("\tLightComponent:\n");
		ss.Add("\t\tColour=").Add(ToString(entity->light.colour)).Add("\n");
		ss.Add("\t\tIntensity=").Add(entity->light.intensity).Add("\n");

		return ss;
	}

	inline static CString SaveRigidBodyComponent(const RigidBodyComponent& ri)
	{
		CString ss = {};

		ss.Add("\t").Add("RigidBody:").Add("\n");
		ss.Add("\t").Add("\t").Add("IMass=").Add(ri.invMass).Add(",COR=").Add(ri.elasticity).Add(",Fric=").Add(ri.friction).Add("\n");
		ss.Add("\t").Add("\t").Add("Forces=").Add(ToString(ri.forces)).Add("\n");
		ss.Add("\t").Add("\t").Add("Torque=").Add(ToString(ri.torque)).Add("\n");
		ss.Add("\t").Add("\t").Add("LinearVelo=").Add(ToString(ri.linearVelocity)).Add("\n");
		ss.Add("\t").Add("\t").Add("AngularVelo=").Add(ToString(ri.angularVelocity)).Add("\n");

		return ss;
	}

#define SAVE_ENTITY_TYPE(type) case EntityType::##type: ss.Add("\tTYPE=").Add(#type); break

	static void SaveWorldSectors(GameState* gs, RenderState* rs, AssetState* as, LargeString& ss)
	{
		ss.Add("World Sectors:\n");
		for (int32 i = 0; i < gs->worldSectorCount; i++)
		{
			WorldSector* sector = &gs->worldSectors[i];
			ss.Add("Sector ").Add(i).Add('\n');
			ss.Add('\t').Add("Boudning Box=").Add(ToString(sector->boundingBox)).Add('\n');
			ss.Add('\t').Add("N1=").Add(sector->neighbour1).Add('\n');
			ss.Add('\t').Add("N2=").Add(sector->neighbour2).Add('\n');
		}
	}

	static void SaveWorldEntities(GameState* gs, RenderState* rs, AssetState* as, LargeString& ss)
	{
		ss.Add("Entities:\n");
		for (int32 i = 0; i < ArrayCount(gs->entites); i++)
		{
			Entity* entity = &gs->entites[i];

			if (IsValidEntity(gs, entity))
			{
				ss.Add("Entity ").Add(i).Add("\n");
				ss.Add("\t").Add("ACTIVE=").Add(entity->active).Add("\n");

				ss.Add("\t").Add("FLAGS=").Add((int32)entity->flags).Add("\n");

				ss.Add("\t").Add("POS=").Add(ToString(entity->transform.position)).Add("\n");
				ss.Add("\t").Add("ORI=").Add(ToString(entity->transform.orientation)).Add("\n");
				ss.Add("\t").Add("SC=").Add(ToString(entity->transform.scale)).Add("\n");

				ss.Add("\t").Add("Colliders=").Add(entity->collision.count).Add("\n");

				ss.Add("\t").Add("\t").Add(ToString(entity->collision.box)).Add("\n");

				ss.Add(SaveRenderComponent(gs, as, rs, entity));
				ss.Add(SaveLightComponent(entity));
				ss.Add(SaveRigidBodyComponent(entity->rigidBody));

				switch (entity->type)
				{
					SAVE_ENTITY_TYPE(ENVIRONMENT);
					SAVE_ENTITY_TYPE(DIR_LIGHT);
					SAVE_ENTITY_TYPE(SPOT_LIGHT);
					SAVE_ENTITY_TYPE(POINT_LIGHT);
				}

				ss.Add("\n");
			}
		}
	}

	static void WriteStringStream(const LargeString& ss, const CString& path)
	{
		PlatformFile file = {};
		file.data = (void*)ss.GetCStr();
		file.size_bytes = ss.GetLength() * sizeof(char);

		DEBUGWriteFile(file, path);
	}

	void SaveWorldPrefabToTextFile(GameState* gs, RenderState* rs, AssetState* as, TransientState* ts, CString path)
	{
		LargeString ss = ts->stringMemory.GetLargeString();
		// @STRING HERE
		SaveWorldEntities(gs, rs, as, ss);
		WriteStringStream(ss, path);

		ts->stringMemory.FreeLargeString();
	}

	void SaveWorldToTextFile(GameState* gs, RenderState* rs, AssetState* as, TransientState* ts, CString path)
	{
		LargeString ss = ts->stringMemory.GetLargeString();
		// @STRING HERE
		SaveWorldSectors(gs, rs, as, ss);
		SaveWorldEntities(gs, rs, as, ss);
		WriteStringStream(ss, path);

		ts->stringMemory.FreeLargeString();
	}

	inline static AABB StringToAABB(const CString& minVec, const CString& maxVec)
	{
		AABB result = {};
		result.min = StringToVec3<real32>(minVec);
		result.max = StringToVec3<real32>(maxVec);

		return result;
	}

	inline static OBB StringToOBB(const CString& str)
	{
		std::vector<CString> values = str.Split(' ');

		OBB result = {};
		for (int32 i = 0; i < 16; i++)
		{
			result.mat.ptr[i] = (values.at(i)).ToReal32();
		}

		CString str_extents = values.at(16);
		result.extents = StringToVec3<real32>(str_extents.Split('=').at(1));

		return result;
	}

	inline static void LoadLightComponent(RenderState* rs, AssetState* as, LargeString& ss, Entity* entity)
	{
		CString line = {};
		line = ss.GetLine();
		line = ss.GetLine();
		entity->light.colour = StringToVec3<real32>(line.Split('=').at(1));
		line = ss.GetLine();
		entity->light.intensity = (line.Split('=').at(1)).ToReal32();
		//entity->object_space_bounding_box = CreateAABBFromCenterRadius(Vec3f(0), Vec3f(0.1f));
	}

	inline static void LoadRenderComponent(RenderState* rs, AssetState* as, LargeString& ss, Entity* entity)
	{
		CString line;
		line = ss.GetLine();

		line = ss.GetLine();
		CString str_meshId = line.Split('=').at(1);
		int32 meshId = 0;
		for (int32 i = 1; i < as->meshCount; i++)
		{
			if (as->meshesData[i].name == str_meshId)
			{
				meshId = i;
				break;
			}
		}

		line = ss.GetLine();
		CString str_textId = line.Split('=').at(1);
		int32 textId = 0;
		for (int32 i = 0; i < as->textureCount; i++)
		{
			if (as->texturesData[i].name == str_textId)
			{
				textId = i;
				break;
			}
		}

		line = ss.GetLine();
		CString str_shaderId = line.Split('=').at(1);
		int32 shaderId = 0;
		for (int32 i = 0; i < as->shaderCount; i++)
		{
			if (as->shadersData[i].name == str_shaderId)
			{
				shaderId = i;
				break;
			}
		}

		line = ss.GetLine();
		CString str_flags = line.Split('=').at(1);
		int32 flags = (str_flags).ToInt32();

		entity->render.flags = flags;
		if (meshId)
		{
			SetEntityMesh(as, entity, meshId);
		}
		if (textId)
		{
			SetEntityTexture(as, entity, textId);
		}
		if (shaderId)
		{
			SetEntityShader(as, entity, shaderId);
		}
	}

	inline static void LoadRigidBodyComponent(RigidBodyComponent* ri, LargeString& ss)
	{
		CString line;
		line = ss.GetLine(); // Remove header
		line = ss.GetLine();
		std::vector<CString> attribs = line.Split(',');
		ri->invMass = attribs.at(0).Split('=').at(1).ToReal32();
		ri->elasticity = attribs.at(1).Split('=').at(1).ToReal32();
		ri->friction = attribs.at(2).Split('=').at(1).ToReal32();

		line = ss.GetLine();
		ri->forces = StringToVec3<real32>(line.Split('=').at(1));

		line = ss.GetLine();
		ri->torque = StringToVec3<real32>(line.Split('=').at(1));

		line = ss.GetLine();
		ri->linearVelocity = StringToVec3<real32>(line.Split('=').at(1));

		line = ss.GetLine();
		ri->angularVelocity = StringToVec3<real32>(line.Split('=').at(1));
	}

	bool32 LoadWorldFromTextFile(GameState* gs, AssetState* as, RenderState* rs, CString path)
	{
		PlatformFile file = DEBUGLoadEntireFile(path, false);

		//path = Util::StringBackSlashesToForward(path);

		bool32 result = true;
		if (file.data)
		{
			CreateEntityFreeList(gs);

			LargeString ss;
			ss.Wrap((char*)file.data, file.size_bytes);

			CString line;

			gs->worldSectorCount = 0;
			while ((line = ss.GetLine()).GetLength() > 0)
			{
				if (line == "Entities:")
				{
					break;
				}

				if (line.StartsWith("Sector"))
				{
					line = ss.GetLine();
					std::vector<CString> strBox = line.Split('=').at(1).Split(' ');
					AABB box = StringToAABB(strBox.at(0), strBox.at(1));

					line = ss.GetLine();
					int32 n1 = line.Split('=').at(1).ToInt32();

					line = ss.GetLine();
					int32 n2 = line.Split('=').at(1).ToInt32();


					gs->worldSectors[gs->worldSectorCount].boundingBox = box;
					gs->worldSectors[gs->worldSectorCount].neighbour1 = n1;
					gs->worldSectors[gs->worldSectorCount].neighbour2 = n2;
					gs->worldSectorCount++;
				}
			}

			while ((line = ss.GetLine()).GetLength() > 0)
			{
				if (line.StartsWith("Entity"))
				{
					Entity* entity = CreateEntity(gs);

					line = ss.GetLine();
					entity->active = (bool32)(line.Split('=').at(1)).ToInt32();

					line = ss.GetLine();
					entity->flags = (EntityFlag)(line.Split('=').at(1)).ToInt32();

					line = ss.GetLine();
					entity->transform.position = StringToVec3<real32>(line.Split('=').at(1));

					line = ss.GetLine();
					entity->transform.orientation = StringToQuat<real32>(line.Split('=').at(1));

					line = ss.GetLine();
					entity->transform.scale = StringToVec3<real32>(line.Split('=').at(1));

					line = ss.GetLine();
					entity->collision.count = (line.Split('=').at(1)).ToInt32();

					// @HACK: Load and save colliders properly
					line = ss.GetLine();
					entity->collision.box = StringToOBB(line);
					entity->collision.type = ColliderType::BOX;

					LoadRenderComponent(rs, as, ss, entity);
					LoadLightComponent(rs, as, ss, entity);
					LoadRigidBodyComponent(&entity->rigidBody, ss);

					line = ss.GetLine();
					entity->type = StringToEntityType(line.Split('=').at(1));
				}
			}
		}
		else
		{
			result = false;
		}

		DEBUGFreeFile(&file);

		return result;
	}

	void EditorPreprocessAllTextures(CString folderPath, TransientState* ts)
	{
		LOG("Preprocessing texture assets");
		PlatformFolder pngFolder = DEBUGLoadEnitreFolder(folderPath, "*.png", false);


		for (int32 pngFileIndex = 0;
			pngFileIndex < pngFolder.files.size();
			pngFileIndex++)
		{
			LargeString ss = ts->stringMemory.GetLargeString();
			// @STRING HERE

			PlatformFile file = pngFolder.files[pngFileIndex];
			CString name = Util::StripFilePathAndExtentions(file.path);

			ss.Add("Flags:\n");
			ss.Add("Mips=0");
			ss.Add("\nData:\n");

			int32 width = -1;
			int32 height = -1;
			int32 channels = -1;
			stbi_set_flip_vertically_on_load(true);

			uint8* pixels = stbi_load_from_memory((stbi_uc*)file.data, (int32)file.size_bytes,
				&width, &height, &channels, 4);

			Assert(pixels, CString("Cannot find image: ").Add(name).GetCStr());

			// @NOTE: We asked stbi to pad it with an aplha value wheather or not it is there.
			// @NOTE: The channel value is the TRUE amount of channel without pad/req comps count.
			// @NOTE: Thus we set it to 4
			channels = 4;

			for (int32 pixelIndex = 0; pixelIndex < width * height; pixelIndex++)
			{
				ss.Add(pixels[pixelIndex]).Add(",");
				ss.Add(pixels[pixelIndex + 1]).Add(",");
				ss.Add(pixels[pixelIndex + 2]).Add(",");
				ss.Add(pixels[pixelIndex + 3]).Add("\n");
			}

			ss.Add("\nEND\n");

			free(pixels);

			PlatformFile dataFile = {};
			dataFile.data = (void*)ss.GetCStr();
			dataFile.size_bytes = ss.GetLength() * sizeof(char);
			DEBUGWriteFile(dataFile, CString("../Assets/Processed/Textures/").Add(name).Add(".txt"));

			ts->stringMemory.FreeLargeString();
		}
	}

	static void PreprocessMesh(TransientState* ts, TemporayMeshBuffer& meshData, TemporyVoxelBuffer& voxBuffer, CString name, uint64 lastWriteTime)
	{
		LargeString slrStream = ts->stringMemory.GetLargeString();
		// @STRING HERE

		slrStream.Add("File Time=").Add(lastWriteTime).Add('\n');

		if (voxBuffer.boxes.size() > 0)
		{
			slrStream.Add("Voxel boxes:\n");
			for (const AABB& box : voxBuffer.boxes)
			{
				slrStream.Add(ToString(box)).Add('\n');
			}
		}
		else
		{
			LOG("Warning " << name.GetCStr() << " doesn't have a corrasponding voxel mesh");
		}


		slrStream.Add("Positions:\n");
		for (int32 i = 0; i < meshData.positions.size(); i++)
		{
			if ((i % 3) == 0 && i != 0)
			{
				slrStream.Add("\n");
			}

			slrStream.Add(ToString(meshData.positions.at(i))).Add(' ');
		}

		slrStream.Add("\nNormals:\n");
		for (int32 i = 0; i < meshData.normals.size(); i++)
		{
			if ((i % 3) == 0 && i != 0)
			{
				slrStream.Add("\n");
			}

			slrStream.Add(ToString(meshData.normals.at(i))).Add(" ");

		}

		slrStream.Add("\nUVs:\n");
		for (int32 i = 0; i < meshData.uvs.size(); i++)
		{
			if ((i % 2) == 0 && i != 0)
			{
				slrStream.Add("\n");
			}

			slrStream.Add(ToString(meshData.uvs.at(i))).Add(" ");
		}

		slrStream.Add("\nPacked:\n");
		for (int32 i = 0; i < meshData.vertices.size(); i++)
		{
			if ((i % 8) == 0 && i != 0)
			{
				slrStream.Add("\n");
			}

			slrStream.Add(meshData.vertices.at(i)).Add(" ");
		}

		slrStream.Add("\nIndices:\n");
		for (int32 i = 0; i < meshData.indices.size(); i++)
		{
			if ((i % 25) == 0 && i != 0)
				slrStream.Add('\n');
			slrStream.Add((uint32)meshData.indices.at(i)).Add(" ");

		}
		slrStream.Add("\nEND\n");

		PlatformFile dataFile = {};
		dataFile.data = (void*)slrStream.GetCStr();
		dataFile.size_bytes = slrStream.GetLength() * sizeof(char);
		DEBUGWriteFile(dataFile, CString("../Assets/Processed/Meshes/").Add(name).Add(".txt"));

		ts->stringMemory.FreeLargeString();

		LOG("Preprocessed: " << name.GetCStr());
	}

	void EditorPreprocessAllMeshes(CString folderPath, TransientState* ts)
	{
		LOG("Preprocessing mesh assets");

		PlatformFolder objFolder = DEBUGLoadEnitreFolder(folderPath, "*.obj", false);
		PlatformFolder voxFolder = DEBUGLoadEnitreFolder(folderPath, "*.ply", false);

		int32 voxFileIndex = 0;
		for (int32 objFileIndex = 0;
			objFileIndex < objFolder.files.size();
			objFileIndex++)
		{
			PlatformFile objFile = objFolder.files[objFileIndex];
			CString objName = Util::StripFilePathAndExtentions(objFile.path);

			PlatformFile voxFile = {};
			CString voxName = {};

			if (voxFileIndex < voxFolder.files.size())
			{
				voxFile = voxFolder.files[voxFileIndex];
				voxName = Util::StripFilePathAndExtentions(voxFile.path);
			}

			TemporayMeshBuffer meshData = DEBUGLoadOBJMesh(objFile);
			TemporyVoxelBuffer voxBuffer = {};
			if (objName == voxName)
			{
				voxBuffer = DEBUGLoadVoxMesh(voxFile);
				OptimizeVoxMesh(&voxBuffer);
				voxFileIndex++;
			}

			PreprocessMesh(ts, meshData, voxBuffer, objName, objFile.lastWriteTime);
		}

		DEBUGFreeFolder(&objFolder);
		DEBUGFreeFolder(&voxFolder);
	}

	void EditorPreprocessOutOfDateMeshes(CString folderPath, TransientState* ts)
	{
		PlatformFolder objFolder = DEBUGLoadEnitreFolder(folderPath, "*.obj", false);
		PlatformFolder voxFolder = DEBUGLoadEnitreFolder(folderPath, "*.ply", false);
		PlatformFolder proFolder = DEBUGLoadEnitreFolder("../Assets/Processed/Meshes/", "*.txt", false);

		std::vector<bool32> filesProcessed(objFolder.files.size());

		for (int32 objFileIndex = 0;
			objFileIndex < objFolder.files.size();
			objFileIndex++)
		{
			PlatformFile objFile = objFolder.files[objFileIndex];
			CString objName = Util::StripFilePathAndExtentions(objFile.path);

			for (int32 proFileIndex = 0;
				proFileIndex < proFolder.files.size();
				proFileIndex++)
			{
				PlatformFile proFile = proFolder.files[proFileIndex];
				CString proName = Util::StripFilePathAndExtentions(proFile.path);

				if (objName == proName)
				{
					LargeString ss;
					ss.Wrap((char*)proFile.data, proFile.size_bytes);

					CString line;
					line = ss.GetLine();

					uint64 fileTime = line.Split('=').at(1).ToUint64();
					if (PlatformCompareFileTimes(objFile.lastWriteTime, fileTime) == 0)
					{
						filesProcessed.at(objFileIndex) = true;
					}

					break;
				}
			}
		}

		for (int32 objFileIndex = 0;
			objFileIndex < objFolder.files.size();
			objFileIndex++)
		{
			if (!filesProcessed.at(objFileIndex))
			{
				PlatformFile objFile = objFolder.files[objFileIndex];
				CString objName = Util::StripFilePathAndExtentions(objFile.path);

				TemporayMeshBuffer meshData = DEBUGLoadOBJMesh(objFile);
				TemporyVoxelBuffer voxBuffer = {};

				for (int32 voxFileIndex = 0;
					voxFileIndex < voxFolder.files.size();
					voxFileIndex++)
				{
					PlatformFile voxFile = voxFolder.files[voxFileIndex];
					CString voxName = Util::StripFilePathAndExtentions(voxFile.path);

					if (objName == voxName)
					{
						voxBuffer = DEBUGLoadVoxMesh(voxFile);
						OptimizeVoxMesh(&voxBuffer);
						break;
					}
				}

				PreprocessMesh(ts, meshData, voxBuffer, objName, objFile.lastWriteTime);
			}
		}

		DEBUGFreeFolder(&voxFolder);
		DEBUGFreeFolder(&objFolder);
		DEBUGFreeFolder(&proFolder);
	}

	void LoadAssetFile(RenderState* rs, AssetState* as, MemoryArena* mem, PlatformFile assetFile)
	{
		LargeString file;
		file.Wrap((char*)assetFile.data, assetFile.size_bytes);

		TemporyVoxelBuffer voxelBuffer;
		TemporayMeshBuffer meshBuffer;

		CString line;

		line = file.GetLine();
		uint64 fileTime = line.Split('=').at(1).ToUint64();

		line = file.GetLine();
		if (line == "Voxel boxes:")
		{
			while ((line = file.GetLine()).GetLength() > 0)
			{
				if (line == "Positions:")
				{
					break;
				}

				std::vector<CString> boxes = line.Split(' ');
				Assert(boxes.size() == 2, "Box count");

				CString strMin = boxes.at(0);
				CString strMax = boxes.at(1);
				AABB box = StringToAABB(strMin, strMax);
				voxelBuffer.boxes.push_back(box);
			}
		}
		else
		{
			line = file.GetLine();
		}

		while ((line = file.GetLine()).GetLength() > 0)
		{
			if (line == "Normals:")
			{
				break;
			}

			std::vector<CString> poses = line.Split(' ');

			for (int32 i = 0; i < poses.size(); i++)
			{
				meshBuffer.positions.push_back(StringToVec3<real32>(poses.at(i)));
			}
		}

		while ((line = file.GetLine()).GetLength() > 0)
		{
			if (line == "UVs:")
			{
				break;
			}

			std::vector<CString> normals = line.Split(' ');

			for (int32 i = 0; i < normals.size(); i++)
			{
				meshBuffer.normals.push_back(StringToVec3<real32>(normals.at(i)));
			}
		}

		while ((line = file.GetLine()).GetLength() > 0)
		{
			if (line == "Packed:")
			{
				break;
			}

			std::vector<CString> uvs = line.Split(' ');

			for (int32 i = 0; i < uvs.size(); i++)
			{
				meshBuffer.uvs.push_back(StringToVec2<real32>(uvs.at(i)));
			}
		}

		while ((line = file.GetLine()).GetLength() > 0)
		{
			if (line == "Indices:")
			{
				break;
			}

			std::vector<CString> vertices = line.Split(' ');

			for (int32 i = 0; i < vertices.size(); i++)
			{
				meshBuffer.vertices.push_back((vertices.at(i).ToReal32()));
			}
		}

		while ((line = file.GetLine()).GetLength() > 0)
		{
			if (line == "END")
			{
				break;
			}

			std::vector<CString> indices = line.Split(' ');

			for (int32 i = 0; i < indices.size(); i++)
			{

				meshBuffer.indices.push_back((indices.at(i).ToInt32()));
			}
		}

		FillMeshData(as, mem, meshBuffer, Util::StripFilePathAndExtentions(assetFile.path));
		FillMeshVoxelData(rs, as, mem, voxelBuffer);
		FillMeshInstance(rs, as, meshBuffer);

		int32 index = as->meshCount;
		as->meshCount++;
	}

	void  EditorLoadProcessedAssets(RenderState* rs, AssetState* as, MemoryArena* mem, CString folderPath)
	{
		LOG("=======================");
		LOG("Loaded assets");
		PlatformFolder assetFolder = DEBUGLoadEnitreFolder(folderPath, "*.txt", false);

		int32 voxFileIndex = 0;
		for (int32 assetIndex = 0;
			assetIndex < assetFolder.files.size();
			assetIndex++)
		{
			PlatformFile assetFile = assetFolder.files.at(assetIndex);
			LOG("Loading " << Util::StripFilePathAndExtentions(assetFile.path).GetCStr());

			LoadAssetFile(rs, as, mem, assetFile);
			LOG("Total completion " << 100.0f * (real32)(assetIndex) / (real32)assetFolder.files.size() << "%")
		}

		DEBUGFreeFolder(&assetFolder);
	}



	struct Font
	{
		CString name; // name of the font
		CString fontImage;
		int size; // size of font, lineheight and baseheight will be based on this as if this is a single unit (1.0)
		float lineHeight; // how far to move down to next line, will be normalized
		float baseHeight; // height of all characters, will be normalized
		int textureWidth; // width of the font texture
		int textureHeight; // height of the font texture
		int numCharacters; // number of characters in the font
		//FontChar* CharList; // list of characters
		int numKernings; // the number of kernings


		// these are how much the character is padded in the texture. We
		// add padding to give sampling a little space so it does not accidentally
		// padd the surrounding characters. We will need to subtract these paddings
		// from the actual spacing between characters to remove the gaps you would otherwise see
		float leftpadding;
		float toppadding;
		float rightpadding;
		float bottompadding;
	};


	/*
	void EditorLoadFontFile(RenderState* rs, CString path)
	{
		std::ifstream fs(path);

		Font font;
		CString tmp;
		int startpos;

		// extract font name
		fs >> tmp >> tmp; // info face="Arial"
		startpos = tmp.find("\"") + 1;
		font.name = tmp.substr(startpos, tmp.size() - startpos - 1);

		// get font size
		fs >> tmp; // size=73
		startpos = tmp.find("=") + 1;
		font.size = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// bold, italic, charset, unicode, stretchH, smooth, aa, padding, spacing
		fs >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp; // bold=0 italic=0 charset="" unicode=0 stretchH=100 smooth=1 aa=1

		// get padding
		fs >> tmp; // padding=5,5,5,5
		startpos = tmp.find("=") + 1;
		tmp = tmp.substr(startpos, tmp.size() - startpos); // 5,5,5,5

		// get up padding
		startpos = tmp.find(",") + 1;
		font.toppadding = std::stoi(tmp.substr(0, startpos)) / (float)windowWidth;

		// get right padding
		tmp = tmp.substr(startpos, tmp.size() - startpos);
		startpos = tmp.find(",") + 1;
		font.rightpadding = std::stoi(tmp.substr(0, startpos)) / (float)windowWidth;

		// get down padding
		tmp = tmp.substr(startpos, tmp.size() - startpos);
		startpos = tmp.find(",") + 1;
		font.bottompadding = std::stoi(tmp.substr(0, startpos)) / (float)windowWidth;

		// get left padding
		tmp = tmp.substr(startpos, tmp.size() - startpos);
		font.leftpadding = std::stoi(tmp) / (float)windowWidth;

		fs >> tmp; // spacing=0,0

		// get lineheight (how much to move down for each line), and normalize (between 0.0 and 1.0 based on size of font)
		fs >> tmp >> tmp; // common lineHeight=95
		startpos = tmp.find("=") + 1;
		font.lineHeight = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)windowHeight;

		// get base height (height of all characters), and normalize (between 0.0 and 1.0 based on size of font)
		fs >> tmp; // base=68
		startpos = tmp.find("=") + 1;
		font.baseHeight = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)windowHeight;

		// get texture width
		fs >> tmp; // scaleW=512
		startpos = tmp.find("=") + 1;
		font.textureWidth = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get texture height
		fs >> tmp; // scaleH=512
		startpos = tmp.find("=") + 1;
		font.textureHeight = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// get pages, packed, page id
		fs >> tmp >> tmp; // pages=1 packed=0
		fs >> tmp >> tmp; // page id=0

		// get texture filename
		CString wtmp;
		fs >> wtmp; // file="Arial.png"
		startpos = wtmp.find("\"") + 1;
		font.fontImage = wtmp.substr(startpos, wtmp.size() - startpos - 1);

		// get number of characters
		fs >> tmp >> tmp; // chars count=97
		startpos = tmp.find("=") + 1;
		font.numCharacters = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// initialize the character list
		font.CharList = new FontChar[font.numCharacters];

		for (int c = 0; c < font.numCharacters; ++c)
		{
			// get unicode id
			fs >> tmp >> tmp; // char id=0
			startpos = tmp.find(L"=") + 1;
			font.CharList[c].id = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

			// get x
			fs >> tmp; // x=392
			startpos = tmp.find(L"=") + 1;
			font.CharList[c].u = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)font.textureWidth;

			// get y
			fs >> tmp; // y=340
			startpos = tmp.find(L"=") + 1;
			font.CharList[c].v = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)font.textureHeight;

			// get width
			fs >> tmp; // width=47
			startpos = tmp.find(L"=") + 1;
			tmp = tmp.substr(startpos, tmp.size() - startpos);
			font.CharList[c].width = (float)std::stoi(tmp) / (float)windowWidth;
			font.CharList[c].twidth = (float)std::stoi(tmp) / (float)font.textureWidth;

			// get height
			fs >> tmp; // height=57
			startpos = tmp.find(L"=") + 1;
			tmp = tmp.substr(startpos, tmp.size() - startpos);
			font.CharList[c].height = (float)std::stoi(tmp) / (float)windowHeight;
			font.CharList[c].theight = (float)std::stoi(tmp) / (float)font.textureHeight;

			// get xoffset
			fs >> tmp; // xoffset=-6
			startpos = tmp.find(L"=") + 1;
			font.CharList[c].xoffset = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)windowWidth;

			// get yoffset
			fs >> tmp; // yoffset=16
			startpos = tmp.find(L"=") + 1;
			font.CharList[c].yoffset = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)windowHeight;

			// get xadvance
			fs >> tmp; // xadvance=65
			startpos = tmp.find(L"=") + 1;
			font.CharList[c].xadvance = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos)) / (float)windowWidth;

			// get page
			// get channel
			fs >> tmp >> tmp; // page=0    chnl=0
		}

		// get number of kernings
		fs >> tmp >> tmp; // kernings count=96
		startpos = tmp.find(L"=") + 1;
		font.numKernings = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

		// initialize the kernings list
		font.KerningsList = new FontKerning[font.numKernings];

		for (int k = 0; k < font.numKernings; ++k)
		{
			// get first character
			fs >> tmp >> tmp; // kerning first=87
			startpos = tmp.find(L"=") + 1;
			font.KerningsList[k].firstid = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

			// get second character
			fs >> tmp; // second=45
			startpos = tmp.find(L"=") + 1;
			font.KerningsList[k].secondid = std::stoi(tmp.substr(startpos, tmp.size() - startpos));

			// get amount
			fs >> tmp; // amount=-1
			startpos = tmp.find(L"=") + 1;
			int t = (float)std::stoi(tmp.substr(startpos, tmp.size() - startpos));
			font.KerningsList[k].amount = (float)t / (float)windowWidth;
		}

		return font;
	}
	*/
}
#endif
