#include "MeshProcessor.h"

namespace cm
{
	MetaProcessor::MetaProcessor()
	{
	}

	CString MetaProcessor::Find(const CString& name) const
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

	void MetaProcessor::LoadAllMetaFiles(const std::vector<CString>& paths)
	{
		metaNames.clear();
		metaPaths.clear();

		for (int32 i = 0; i < paths.size(); i++)
		{
			metaNames.push_back(Util::StripFilePathAndExtentions(paths.at(i)));
		}

		metaPaths = paths;
	}

	std::vector<CString> MetaProcessor::FindMissing(const std::vector<CString>& paths)
	{
		std::vector<CString> missing;
		for (const CString& path : paths)
		{
			if (Find(Util::StripFilePathAndExtentions(path)).GetLength() == 0)
			{
				missing.push_back(path);
			}
		}

		return missing;
	}

	void MetaProcessor::SaveMetaData(const CString& path, const ModelMetaFile& modelData)
	{
		TextFileWriter file;
		file.WriteLine(CString("GUID=").Add(GenerateAssetId()));
		file.WriteLine(CString("Type=MODEL"));
		file.WriteLine(CString("Scale=").Add(modelData.scale));
		file.WriteLine(CString("Layout=").Add(modelData.layout.ToString()));

		file.SaveToDisk(path);
	}

	ModelMetaFile MetaProcessor::ParseModelMetaFile(const CString& path) const
	{
		TextFileReader file;
		file.Read(path);

		ModelMetaFile metaFile = {};

		for (CString line = file.NextLine(); line != TextFileReader::END_OF_FILE_STRING; line = file.NextLine())
		{
			if (line.StartsWith("GUID"))
			{
				metaFile.id = line.SubStr(line.FindFirstOf('=') + 1).ToUint64();
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
				metaFile.layout = VertexShaderLayoutType::ValueOf(line.SubStr(line.FindFirstOf('=') + 1));
			}
		}

		return metaFile;
	}


}