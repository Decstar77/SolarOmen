#pragma once
#include "Core.h"
#include "core/SolarTypes.h"
#include "TextFile.h"
#include "MetaProcessor.h"
#include "BinaryFile.h"


namespace cm
{
	struct ShaderCollection
	{
		CString vertexPath;
		CString pixelPath;
		CString computePath;
	};

	class Program : public Serializable
	{
	public:
		Program(const ShaderCollection& shaderCollection, AssetId id, const CString& name)
			: id(id), name(name)
		{
			LoadShader(shaderCollection);
		}

		AssetId id;
		CString name;
		ProgramStagesLayout stagesLayout;
		VertexLayoutType vertexLayout;
		std::vector<char> vertexData;
		std::vector<char> computeData;
		std::vector<char> pixelData;

		void LoadShader(const ShaderCollection& path);
		virtual void SaveBinaryData(BinaryFile* file) const override;
	};

	class ProgramProcessor
	{
	public:
		std::vector<Program> LoadPrograms(const std::vector<CString>& shaderPaths);
	};
}