#pragma once
#include "Core.h"
#include "core/SolarAssets.h"
#include "TextFile.h"
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

	class MetaFileProcessor
	{
	public:
		std::vector<CString> metaPaths;
		std::vector<CString> metaNames;

		MetaFileProcessor(const std::vector<CString>& paths)
		{
			for (int32 i = 0; i < paths.size(); i++)
			{
				metaNames.push_back(Util::StripFilePathAndExtentions(paths.at(i)));
			}

			metaPaths = paths;
		}

		CString Find(const CString& name) const;

		DISABLE_COPY_AND_MOVE(MetaFileProcessor);
	};

	class FatVertex : public Serializable
	{
	public:
		inline static constexpr uint32 MAX_BONE_INFLUENCE = 4;

		Vec3f position;
		Vec3f normal;
		Vec2f texCoords;
		Vec3f tangent;
		Vec3f bitangent;
		Vec4f colours;
		int32 boneIds[MAX_BONE_INFLUENCE];
		real32 boneWeights[MAX_BONE_INFLUENCE];

		virtual void SaveBinaryData(BinaryFile* file) const override;
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

		CString materialName;

		std::vector<FatVertex> vertices;
		std::vector<uint32> indices;

		std::vector<Vec3f> positions;
		std::vector<Vec3f> normals;
		std::vector<Vec2f> uvs;
		std::vector<Vec3f> tangets;
		std::vector<Vec3f> bitangets;
		std::vector<Vec4f> colours;
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

	private:
		AssetId id;
		CString name;
		std::vector<Mesh> meshes;

		void LoadModel(const CString& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	};

	class ModelProcessor
	{
	public:
		ModelProcessor();

		std::vector<Model> LoadModels(const std::vector<CString>& modelPaths, const MetaFileProcessor& metaProcessor);
		void SaveModels(const std::vector<Model>& models);

		DISABLE_COPY_AND_MOVE(ModelProcessor);
	};
}
