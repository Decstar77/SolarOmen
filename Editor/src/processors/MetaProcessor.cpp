#include "ModelProcessor.h"

namespace sol
{
	MetaProcessor::MetaProcessor()
	{
	}

	String MetaProcessor::Find(const String& name) const
	{
		for (int32 i = 0; i < metaNames.size(); i++)
		{
			if (metaNames.at(i) == name)
			{
				return metaPaths.at(i);
			}
		}

		return "";
	}

	void MetaProcessor::LoadAllMetaFiles(const std::vector<String>& paths)
	{
		metaNames.clear();
		metaPaths.clear();

		for (int32 i = 0; i < paths.size(); i++)
		{
			metaNames.push_back(Util::StripFilePathAndExtentions(paths.at(i)));
		}

		metaPaths = paths;
	}

	std::vector<String> MetaProcessor::FindMissing(const std::vector<String>& paths)
	{
		std::vector<String> missing;
		for (const String& path : paths)
		{
			if (Find(Util::StripFilePathAndExtentions(path)).GetLength() == 0)
			{
				missing.push_back(path);
			}
		}

		return missing;
	}

	void MetaProcessor::SaveMetaData(const String& path, const ModelMetaFile& modelData)
	{
		TextFileWriter file;
		file.WriteLine(String("GUID=").Add(GenerateAssetId()));
		file.WriteLine(String("Type=MODEL"));
		file.WriteLine(String("Scale=").Add(modelData.scale));
		file.WriteLine(String("Layout=").Add(modelData.layout.ToString()));

		file.SaveToDisk(path);
	}

	void MetaProcessor::SaveMetaData(const String& path, const TextureMetaFile& textureData)
	{
		TextFileWriter file;

		if (textureData.id.IsValid()) { file.WriteLine(String("GUID=").Add(textureData.id)); }
		else { file.WriteLine(String("GUID=").Add(GenerateAssetId())); }

		file.WriteLine(String("Type=TEXTURE"));
		file.WriteLine(String("Format=").Add(textureData.format.ToString()));
		file.WriteLine(String("Usage0=").Add(textureData.usage[0].ToString()));
		file.WriteLine(String("Usage1=").Add(textureData.usage[1].ToString()));
		file.WriteLine(String("Usage2=").Add(textureData.usage[2].ToString()));
		file.WriteLine(String("Usage3=").Add(textureData.usage[3].ToString()));
		file.WriteLine(String("CPUFlags=").Add(textureData.cpuFlags.ToString()));
		file.WriteLine(String("Mips=").Add((int32)textureData.mips));
		file.WriteLine(String("IsSkybox=").Add((int32)textureData.isSkybox));
		file.WriteLine(String("IsNormalMap=").Add((int32)textureData.isNormalMap));

		file.SaveToDisk(path);
	}

	ModelMetaFile MetaProcessor::ParseModelMetaFile(const String& path) const
	{
		TextFileReader file;
		file.Read(path);

		ModelMetaFile metaFile = {};

		for (String line = file.NextLine(); line != TextFileReader::END_OF_FILE_STRING; line = file.NextLine())
		{
			if (line.StartsWith("GUID"))
			{
				metaFile.id.number = line.SubStr(line.FindFirstOf('=') + 1).ToUint64();
			}
			else if (line.StartsWith("Type"))
			{

			}
			else if (line.StartsWith("Scale"))
			{
				metaFile.scale = line.SubStr(line.FindFirstOf('=') + 1).ToReal32();
			}
			else if (line.StartsWith("Layout"))
			{
				metaFile.layout = VertexLayoutType::ValueOf(line.SubStr(line.FindFirstOf('=') + 1));
			}
		}

		return metaFile;
	}

	TextureMetaFile MetaProcessor::ParseTextureMetaFile(const String& path) const
	{
		TextFileReader file;
		file.Read(path);

		TextureMetaFile metaFile = {};
		metaFile.format = TextureFormat::Value::R8G8B8A8_UNORM;
		metaFile.usage[0] = BindUsage::Value::SHADER_RESOURCE;

		for (String line = file.NextLine(); line != TextFileReader::END_OF_FILE_STRING; line = file.NextLine())
		{
			if (line.StartsWith("GUID"))
			{
				metaFile.id.number = line.SubStr(line.FindFirstOf('=') + 1).ToUint64();
			}
			else if (line.StartsWith("Type"))
			{
			}
			else if (line.StartsWith("Mips"))
			{
				metaFile.mips = (bool8)line.SubStr(line.FindFirstOf('=') + 1).ToInt32();
			}
			else if (line.StartsWith("IsSkybox"))
			{
				metaFile.isSkybox = (bool8)line.SubStr(line.FindFirstOf('=') + 1).ToInt32();
			}
			else if (line.StartsWith("IsNormalMap"))
			{
				metaFile.isNormalMap = (bool8)line.SubStr(line.FindFirstOf('=') + 1).ToInt32();
			}
			else if (line.StartsWith("Scale"))
			{
			}
			else if (line.StartsWith("Layout"))
			{
			}
		}

		return metaFile;
	}
}