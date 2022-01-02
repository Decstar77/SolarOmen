#pragma once
#include "Core.h"
#include "TextFile.h"
#include "BinaryFile.h"
#include "core/SolarTypes.h"
#include "AssetIdGenerator.h"

namespace cm
{
	class Material : Serializable
	{
	public:
		AssetId id;
		CString name;
		Vec3f colourKd;

		virtual void SaveBinaryData(BinaryFile* file) const override;
	};

	class MaterialProcessor
	{
	public:
		std::vector<Material> LoadMTLMaterials(const std::vector<CString>& paths);

	};
}