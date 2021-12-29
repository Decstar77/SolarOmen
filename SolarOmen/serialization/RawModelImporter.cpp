#include "RawModelImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace cm
{
	static ModelAsset ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		ModelAsset meshData = {};

		meshData.indices = GameMemory::PushPermanentArray<uint32>(mesh->mNumFaces * 3);
		uint32 packedCount = 3 * mesh->mNumVertices + 3 * mesh->mNumVertices + 2 * mesh->mNumVertices;
		meshData.packedVertices = GameMemory::PushPermanentArray<real32>(packedCount);
		meshData.layout = VertexShaderLayoutType::Value::PNT;

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

			Assert(counter <= (uint32)packedCount, "To many vertices");
		}

		for (uint32 i = 0, counter = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			Assert(face.mNumIndices == 3, "Face not triangle");

			for (uint32 j = 0; j < face.mNumIndices; j++)
			{
				Assert(counter < (uint32)meshData.indices.GetCapcity(), "To many indicies");
				meshData.indices[counter] = face.mIndices[j];
				counter++;
			}
		}

		meshData.indices.count = meshData.indices.GetCapcity();
		meshData.packedVertices.count = meshData.packedVertices.GetCapcity();

		return meshData;
	}

	static ModelAsset ProcessNode(aiNode* node, const aiScene* scene)
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

	ModelAsset LoadModel(CString filePath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath.GetCStr(), aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG(importer.GetErrorString());
			return {};
		}

		ModelAsset mesh = ProcessNode(scene->mRootNode, scene);

		importer.FreeScene();

		return mesh;
	}
}