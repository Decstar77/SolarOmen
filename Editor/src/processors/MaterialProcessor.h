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

		ResourceId abledoTexture;
		String abledoTextureName;

		ResourceId occlusionRoughnessMetallicTexture;
		String occlusionRoughnessMetallicTextureName;

		ResourceId normalTexture;
		String normalTextureName;

		ResourceId emssiveTexture;
		String emssiveTextureName;

		Vec3f colourKd;

		virtual bool8 SaveBinaryData(BinaryFile* file) const override;
	};

	class MaterialProcessor
	{
	public:
		std::vector<MeshMaterial> LoadMTLMaterials(const std::vector<String>& paths);

	};
}