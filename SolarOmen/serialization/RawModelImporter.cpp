#include "RawModelImporter.h"
#include "../SolarAssets.h"
#include "core/SolarCore.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace cm
{
	static MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		MeshData meshData = {};

		meshData.positionCount = mesh->mNumVertices;
		meshData.positions = GameMemory::PushPermanentCount<Vec3f>(mesh->mNumVertices);
		meshData.normalCount = mesh->mNumVertices;
		meshData.normals = GameMemory::PushPermanentCount<Vec3f>(mesh->mNumVertices);
		meshData.uvCount = mesh->mNumVertices;
		meshData.uvs = GameMemory::PushPermanentCount<Vec2f>(mesh->mNumVertices);

		meshData.indicesCount = mesh->mNumFaces * 3;
		meshData.indices = GameMemory::PushPermanentCount<uint32>(meshData.indicesCount);

		meshData.packedStride = 3 + 3 + 2;
		meshData.packedCount = 3 * meshData.positionCount + 3 * meshData.normalCount + 2 * meshData.uvCount;
		meshData.packedVertices = GameMemory::PushPermanentCount<real32>(meshData.packedCount);

		for (uint32 i = 0, counter = 0; i < mesh->mNumVertices; i++)
		{
			Vec3f vertex;
			vertex.x = mesh->mVertices[i].x;
			vertex.y = mesh->mVertices[i].y;
			vertex.z = mesh->mVertices[i].z;

			Vec3f normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;

			Vec2f uv = Vec2f(0);
			if (mesh->mTextureCoords[0])
			{
				uv.x = mesh->mTextureCoords[0][i].x;
				uv.y = mesh->mTextureCoords[0][i].y;
			}


			meshData.positions[i] = vertex;
			meshData.normals[i] = normal;
			meshData.uvs[i] = uv;

			meshData.packedVertices[counter] = vertex.x;
			counter++;
			meshData.packedVertices[counter] = vertex.y;
			counter++;
			meshData.packedVertices[counter] = vertex.z;
			counter++;
			meshData.packedVertices[counter] = normal.x;
			counter++;
			meshData.packedVertices[counter] = normal.y;
			counter++;
			meshData.packedVertices[counter] = normal.z;
			counter++;
			meshData.packedVertices[counter] = uv.x;
			counter++;
			meshData.packedVertices[counter] = uv.y;
			counter++;

			Assert(counter <= (uint32)meshData.packedCount, "To many vertices");
		}

		for (uint32 i = 0, counter = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			Assert(face.mNumIndices == 3, "Face not triangle");

			for (uint32 j = 0; j < face.mNumIndices; j++)
			{
				Assert(counter < (uint32)meshData.indicesCount, "To many indicies");
				meshData.indices[counter] = face.mIndices[j];
				counter++;
			}
		}

		return meshData;
	}

	static MeshData ProcessNode(aiNode* node, const aiScene* scene)
	{
		if (node->mNumMeshes > 0)
		{
			Assert(node->mNumMeshes == 1, "We don't support multi mesh objects");
			aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
			return ProcessMesh(mesh, scene);
		}

		if (node->mNumChildren > 0)
		{
			Assert(node->mNumChildren == 1, "We don't support multi mesh objects");
			return ProcessNode(node->mChildren[0], scene);
		}

		Assert(0, "");
		return {};
	}

	MeshData LoadModel(CString filePath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath.GetCStr(), aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG(importer.GetErrorString());
			return {};
		}

		MeshData mesh = ProcessNode(scene->mRootNode, scene);

		importer.FreeScene();

		return mesh;
	}
}