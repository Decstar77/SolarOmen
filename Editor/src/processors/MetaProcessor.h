#pragma once

#include "../Core.h"
#include "TextFile.h"
#include "AssetIdGenerator.h"

namespace sol
{
	enum class MetaFileType
	{
		INVALID = 0,
		MODEL,
		TEXTURE,
		SHADER,
		AUDIO,
		FONT,
		ROOM,
		COUNT
	};

	struct ModelMetaFile
	{
		ResourceId id;
		real32 scale;
		VertexLayoutType layout;
	};

	struct TextureMetaFile
	{
		ResourceId id;
		TextureFormat format;
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;
		bool8 mips;
		bool8 isSkybox;
		bool8 isNormalMap;
	};

	class MetaProcessor
	{
	public:
		std::vector<String> metaPaths;
		std::vector<String> metaNames;

		MetaProcessor();
		void LoadAllMetaFiles(const std::vector<String>& paths);
		String Find(const String& name) const;
		std::vector<String> FindMissing(const std::vector<String>& paths);

		void SaveMetaData(const String& path, const ModelMetaFile& modelData);
		void SaveMetaData(const String& path, const TextureMetaFile& textureData);

		ModelMetaFile ParseModelMetaFile(const String& path) const;
		TextureMetaFile ParseTextureMetaFile(const String& path) const;

		DISABLE_COPY_AND_MOVE(MetaProcessor);
	};

}

