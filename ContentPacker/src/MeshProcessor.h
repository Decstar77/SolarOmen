#pragma once
#include "Core.h"
#include "core/SolarAssets.h"
#include "TextFile.h"
#include "MetaProcessor.h"
#include "BinaryFile.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace cm
{
#pragma pack(push, 1)

	struct AssetHeader
	{
		AssetId id;
		CString name;
	};

#pragma pack(pop)


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

		CString materialName;

		std::vector<FatVertex> vertices;
		std::vector<uint32> indices;
	};

	class Model : public Serializable
	{
	public:
		Model(const CString& path, AssetId id, const CString& name)
			: id(id), name(name)
		{
			LoadModel(path);
		}

		virtual void SaveBinaryData(BinaryFile* file) const override;

		AssetId id;
		CString name;
		std::vector<Mesh> meshes;

	private:
		void LoadModel(const CString& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	};

	class ModelProcessor
	{
	public:
		ModelProcessor();

		std::vector<Model> LoadModels(const std::vector<CString>& modelPaths, const MetaProcessor& metaProcessor);
		void SaveModels(const std::vector<Model>& models);

		DISABLE_COPY_AND_MOVE(ModelProcessor);
	};
}
