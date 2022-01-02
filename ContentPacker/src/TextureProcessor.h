#pragma once

#include "Core.h"
#include "TextFile.h"
#include "BinaryFile.h"
#include "AssetIdGenerator.h"
#include "MetaProcessor.h"

namespace cm
{
	class Texture : public Serializable
	{
	public:
		Texture(const CString& path, AssetId id, const CString& name)
			: id(id), name(name)
		{
			mips = 0;
			width = 0;
			height = 0;
			LoadTexture(path);
		}

		AssetId id;
		CString name;
		uint8 mips;
		int32 width;
		int32 height;
		TextureFormat format;
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;
		std::vector<uint8> pixels;

		void LoadTexture(const CString& path);

		virtual void SaveBinaryData(BinaryFile* file) const override;
	};

	class TextureProcessor
	{
	public:
		TextureProcessor();
		std::vector<Texture> LoadTextures(const std::vector<CString>& texturePaths, const MetaProcessor& metaProcessor);
	};
}