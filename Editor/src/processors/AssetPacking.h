#pragma once
#include "../Core.h"
#include "FileProcessor.h"
#include "MetaProcessor.h"
#include "ModelProcessor.h"
#include "TextureProcessor.h"

namespace sol
{
	template<typename T>
	void SaveBinaryData(const std::vector<T>& data, const String& path)
	{
		BinaryFile file;
		file.Write((uint32)data.size());
		for (const T& d : data)
		{
			d.SaveBinaryData(&file);
		}
		file.SaveToDisk(path);
	}

	std::vector<Model> LoadAndProcessModels(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor);
	std::vector<Texture> LoadAndProcessTextures(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor);
}
