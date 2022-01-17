#pragma once

#include "../Core.h"
#include "TextFile.h"
#include "BinaryFile.h"
#include "AssetIdGenerator.h"
#include "MetaProcessor.h"

namespace sol
{
	class Texture : public Serializable
	{
	public:
		Texture(const String& path, ResourceId id, const String& name)
			: id(id), name(name)
		{
			mips = 0;
			width = 0;
			height = 0;
			LoadTexture(path);
		}

		ResourceId id;
		String name;
		uint8 mips;
		int32 width;
		int32 height;
		TextureFormat format;
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;
		std::vector<uint8> pixels;

		void LoadTexture(const String& path);

		virtual bool8 SaveBinaryData(BinaryFile* file) const override;
	};

	class TextureProcessor
	{
	public:
		TextureProcessor();
		std::vector<Texture> LoadTextures(const std::vector<String>& texturePaths, const MetaProcessor& metaProcessor);
	};
}