#include "MaterialProcessor.h"

namespace cm
{
	std::vector<CString> SplitString(const CString& str, const char& delim)
	{
		const int32 len = str.GetLength();
		std::vector<CString> result;

		int32 start = 0;
		int32 end = 0;
		for (; end < len; end++)
		{
			if (str[end] == delim)
			{
				if (start != end)
				{
					CString r = "";
					r.CopyFrom(str, start, end - 1);
					result.push_back(r);
					start = end + 1;
				}
				else
				{
					start++;
				}
			}
		}

		if (end != start)
		{
			CString r = "";
			r.CopyFrom(str, start, end - 1);
			result.push_back(r);
		}

		return result;
	}

	void Material::SaveBinaryData(BinaryFile* file) const
	{
		file->Write(id);
		file->Write(name);
		file->Write(colourKd);
	}

	std::vector<Material> MaterialProcessor::LoadMTLMaterials(const std::vector<CString>& paths)
	{
		std::vector<Material> materials;

		for (const CString& path : paths)
		{
			TextFileReader fileReader;
			fileReader.Read(path);

			CString line = fileReader.NextLine();
			while (line != TextFileReader::END_OF_FILE_STRING)
			{
				if (line.StartsWith("newmtl"))
				{
					Material material;
					line = line.SubStr(line.FindFirstOf(' ') + 1);
					material.name = line;

					line = fileReader.NextLine();
					if (line.StartsWith("Kd"))
					{
						line = line.SubStr(line.FindFirstOf(' ') + 1);
						std::vector<CString> values = SplitString(line, ' ');
						real32 x = values[0].ToReal32();
						real32 y = values[1].ToReal32();
						real32 z = values[2].ToReal32();

						material.colourKd = Vec3f(x, y, z);
					}

					bool add = true;
					for (const Material& mat : materials)
					{
						if (mat.name == material.name)
						{
							add = false;
							break;
						}
					}

					if (add)
					{
						materials.push_back(material);
					}
				}

				line = fileReader.NextLine();
			}
		}

		return materials;
	}
}