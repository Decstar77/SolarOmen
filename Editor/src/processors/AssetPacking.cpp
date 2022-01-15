#include "AssetPacking.h"


namespace sol
{
	std::vector<Model> LoadAndProcessModels(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
	{
		std::vector<String> paths = fileProcessor.GetFilePaths(path, "gltf");
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

		metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(path, "slo"));

		SOLTRACE("PROCESSING MODELS");
		ModelProcessor modelProcessor;
		std::vector<Model> models = modelProcessor.LoadModels(paths, metaProcessor);
		SOLTRACE("COMPLETE");

		return models;
	}

	std::vector<Texture> LoadAndProcessTextures(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
	{
		std::vector<String> paths = fileProcessor.GetFilePaths(path, "png");
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

		metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(path, "slo"));

		SOLTRACE("PROCESSING TEXTURES");
		TextureProcessor textureProcessor;
		std::vector<Texture> textures = textureProcessor.LoadTextures(paths, metaProcessor);
		SOLTRACE("COMPLETE");

		return textures;
	}
}