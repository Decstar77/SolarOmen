#pragma once

#include "Core.h"
#include "TextFile.h"
#include "AssetIdGenerator.h"

namespace cm
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
		AssetId id;
		real32 scale;
		VertexLayoutType layout;
	};

	struct TextureMetaFile
	{
		AssetId id;
		TextureFormat format;
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;
		bool mips;
	};

	class MetaProcessor
	{
	public:
		std::vector<CString> metaPaths;
		std::vector<CString> metaNames;

		MetaProcessor();
		void LoadAllMetaFiles(const std::vector<CString>& paths);
		CString Find(const CString& name) const;
		std::vector<CString> FindMissing(const std::vector<CString>& paths);

		void SaveMetaData(const CString& path, const ModelMetaFile& modelData);
		void SaveMetaData(const CString& path, const TextureMetaFile& textureData);

		ModelMetaFile ParseModelMetaFile(const CString& path) const;
		TextureMetaFile ParseTextureMetaFile(const CString& path) const;

		DISABLE_COPY_AND_MOVE(MetaProcessor);
	};

}

