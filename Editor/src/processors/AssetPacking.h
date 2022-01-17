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

		bool8 allFine = true;

		for (const T& d : data)
		{
			allFine = d.SaveBinaryData(&file);
			if (!allFine) { SOLERROR("Could not save file due to Seriziable failure"); return; }
		}
		file.SaveToDisk(path);
	}

	std::vector<Model> LoadAndProcessModels(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor);
	std::vector<Texture> LoadAndProcessTextures(String path, FileProcessor& fileProcessor, MetaProcessor& metaProcessor);
}
