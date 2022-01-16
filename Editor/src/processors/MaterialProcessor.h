#pragma once
#include "../Core.h"
#include "TextFile.h"
#include "BinaryFile.h"
#include "AssetIdGenerator.h"

namespace sol
{
	class MeshMaterial : Serializable
	{
	public:
		ResourceId id;
		String name;

		String abledoTextureName;
		String occlusionRoughnessMetallicTextureName;
		String normalTextureName;
		String emssiveTextureName;

		Vec3f colourKd;

		virtual void SaveBinaryData(BinaryFile* file) const override;
	};

	class MaterialProcessor
	{
	public:
		std::vector<MeshMaterial> LoadMTLMaterials(const std::vector<String>& paths);

	};
}