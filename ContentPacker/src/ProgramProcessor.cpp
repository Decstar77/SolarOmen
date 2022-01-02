#include "ProgramProcessor.h"
#include <unordered_map>

namespace cm
{
	void Program::LoadShader(const ShaderCollection& shaderCollection)
	{
		BinaryFileReader reader;

		vertexLayout = VertexLayoutType::Value::PNT;
		bool vertex = false;
		bool pixel = false;
		bool compute = false;

		if (shaderCollection.vertexPath.GetLength() > 0) { vertexData = reader.Read(shaderCollection.vertexPath); vertex = true; }
		if (shaderCollection.pixelPath.GetLength() > 0) { pixelData = reader.Read(shaderCollection.pixelPath); pixel = true; }
		if (shaderCollection.computePath.GetLength() > 0) { computeData = reader.Read(shaderCollection.computePath); compute = true; }

		if (vertex && pixel)
		{
			stagesLayout = ProgramStagesLayout::Value::VERTEX_PIXEL;
		}
		else if (compute)
		{
			stagesLayout = ProgramStagesLayout::Value::COMPUTE;
		}
		else
		{
			Assert(0, "ProgramStagesLayout");
		}
	}

	void Program::SaveBinaryData(BinaryFile* file) const
	{
		file->Write(id);
		file->Write(name);
		file->Write((uint8)stagesLayout.Get());
		file->Write((uint8)vertexLayout.Get());
		if (vertexData.size() > 0) { file->Write(vertexData); }
		if (pixelData.size() > 0) { file->Write(pixelData); }
		if (computeData.size() > 0) { file->Write(computeData); }
	}

	std::vector<Program> ProgramProcessor::LoadPrograms(const std::vector<CString>& shaderPaths)
	{
		std::unordered_map<std::string, ShaderCollection> shaders;

		for (const CString& path : shaderPaths)
		{
			std::string name = Util::StripFilePathAndExtentions(path).GetCStr();
			if (shaders.find(name) == shaders.end()) { shaders[name] = {}; }

			ShaderCollection* shader = &shaders.at(name);
			CString ext = Util::GetFileExtension(path);
			if (ext.StartsWith("vert")) { shaders.at(name).vertexPath = path; }
			else if (ext.StartsWith("pixl")) { shaders.at(name).pixelPath = path; }
			else if (ext.StartsWith("comp")) { shaders.at(name).computePath = path; }
			else { Assert(0, "Unkwown shader type"); }
		}

		std::vector<Program> programs;
		for (auto it : shaders)
		{
			const ShaderCollection& shaderCollection = it.second;
			AssetId id = GenerateAssetId();
			CString name = it.first.c_str();
			programs.emplace_back(shaderCollection, id, name);
		}

		return programs;
	}
}