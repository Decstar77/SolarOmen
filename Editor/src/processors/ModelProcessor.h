#pragma once
#include "../Core.h"
#include "TextFile.h"
#include "MetaProcessor.h"
#include "BinaryFile.h"
#include "MaterialProcessor.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace sol
{
#pragma pack(push, 1)

	struct AssetHeader
	{
		ResourceId id;
		String name;
	};

#pragma pack(pop)

	struct FatVertex
	{
		inline static constexpr uint32 MAX_BONE_INFLUENCE = 4;

		Vec3f position;
		Vec3f normal;
		Vec2f texCoords;
		Vec3f tangent;
		Vec3f bitangent;
		Vec4f colours;
		int32 boneIds[MAX_BONE_INFLUENCE] = {};
		real32 boneWeights[MAX_BONE_INFLUENCE] = {};
	};

	class Mesh
	{
	public:
		Mesh()
		{
			hasIndices = true;
			hasNormals = true;
			hasColours = false;
			hasUVs = true;
			hasTangets = true;
			hastBitangets = true;
		}

		bool hasIndices;
		bool hasNormals;
		bool hasUVs;
		bool hasColours;
		bool hasTangets;
		bool hastBitangets;

		String name;
		MeshMaterial material;
		std::vector<FatVertex> vertices;
		std::vector<uint32> indices;
	};

	class Model : public Serializable
	{
	public:
		Model(const String& path, const String name, ModelMetaFile metaData)
			: name(name), metaData(metaData)
		{
			LoadModel(path);
		}

		virtual void SaveBinaryData(BinaryFile* file) const override;


		String name;
		std::vector<Mesh> meshes;
		ModelMetaFile metaData;
	private:
		void LoadModel(const String& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	};

	class ModelProcessor
	{
	public:
		ModelProcessor();

		std::vector<Model> LoadModels(const std::vector<String>& modelPaths, const MetaProcessor& metaProcessor);

		DISABLE_COPY_AND_MOVE(ModelProcessor);
	};
}
