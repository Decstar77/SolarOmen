#pragma once

#include "Core.h"
#include "TextFile.h"
#include "BinaryFile.h"
#include "AssetIdGenerator.h"
#include "MetaProcessor.h"

namespace cm
{
	class Texture
	{
	public:
		AssetId id;
		CString name;
		TextureFormat format;
		BindUsage usage[4];
		ResourceCPUFlags cpuFlags;
		int32 width;
		int32 height;
		std::vector<uint8> pixels;
		bool32 mips;
	};

	class TextureProcessor
	{
	};
}