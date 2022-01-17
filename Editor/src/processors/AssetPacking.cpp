#include "AssetPacking.h"


namespace sol
{
	void CreateMissingModelMetaFiles(const std::vector<String>& paths, FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
	{
		std::vector<String> missingMetaFiles = metaProcessor.FindMissing(paths);

		for (const String& path : missingMetaFiles)
		{
			ModelMetaFile metaData = {};
			metaData.scale = 1.0f;
			metaData.layout = VertexLayoutType::Value::PNT;

			String newPath = Util::StripFileExtension(path).Add(".slo");
			metaProcessor.SaveMetaData(newPath, metaData);
			SOLTRACE(String("Creating meta file for: ").Add(path.GetCStr()).GetCStr());
		}
	}

	void CreateMissingTextureMetaFiles(std::vector<String> paths, FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
	{
		std::vector<String> missingMetaFiles = metaProcessor.FindMissing(paths);

		for (const String& path : missingMetaFiles)
		{
			TextureMetaFile metaData = {};
			metaData.format = TextureFormat::Value::R8G8B8A8_UNORM;
			metaData.usage[0] = BindUsage::Value::SHADER_RESOURCE;
			metaData.mips = false;

			String newPath = Util::StripFileExtension(path).Add(".slo");
			metaProcessor.SaveMetaData(newPath, metaData);

			SOLTRACE(String("Creating meta file for: ").Add(path.GetCStr()).GetCStr());
		}
	}

	std::vector<Model> LoadAndProcessModels(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
	{
		std::vector<String> meshPaths = fileProcessor.GetFilePaths(path, "obj");
		std::vector<String> texturePaths = CombineStdVectors(fileProcessor.GetFilePaths(path, "jpg"), fileProcessor.GetFilePaths(path, "png"));

		CreateMissingModelMetaFiles(meshPaths, fileProcessor, metaProcessor);
		CreateMissingTextureMetaFiles(texturePaths, fileProcessor, metaProcessor);

		metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(path, "slo"));

		SOLTRACE("PROCESSING MODELS");
		ModelProcessor modelProcessor;
		std::vector<Model> models = modelProcessor.LoadModels(meshPaths, metaProcessor);

		for (Model& model : models)
		{
			for (uint32 i = 0; i < model.meshes.size(); i++)
			{
				for (uint32 j = i + 1; j < model.meshes.size(); j++)
				{
					Mesh* a = &model.meshes[i];
					Mesh* b = &model.meshes[j];

					if (a->material.abledoTextureName == b->material.abledoTextureName)
					{
						int ea = 2;
					}
				}
			}
		}


		SOLTRACE("COMPLETE");

		return models;
	}

	std::vector<Texture> LoadAndProcessTextures(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
	{
		std::vector<String> paths = CombineStdVectors(fileProcessor.GetFilePaths(path, "jpg"), fileProcessor.GetFilePaths(path, "png"));
		CreateMissingTextureMetaFiles(paths, fileProcessor, metaProcessor);
		metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(path, "slo"));

		SOLTRACE("PROCESSING TEXTURES");
		TextureProcessor textureProcessor;
		std::vector<Texture> textures = textureProcessor.LoadTextures(paths, metaProcessor);
		SOLTRACE("COMPLETE");

		return textures;
	}
}