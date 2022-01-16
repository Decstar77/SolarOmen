#include "MaterialProcessor.h"

namespace sol
{
	std::vector<String> SplitString(const String& str, const char& delim)
	{
		const int32 len = str.GetLength();
		std::vector<String> result;

		int32 start = 0;
		int32 end = 0;
		for (; end < len; end++)
		{
			if (str[end] == delim)
			{
				if (start != end)
				{
					String r = "";
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
			String r = "";
			r.CopyFrom(str, start, end - 1);
			result.push_back(r);
		}

		return result;
	}

	void MeshMaterial::SaveBinaryData(BinaryFile* file) const
	{
		file->Write(id);
		file->Write(name);
		file->Write(colourKd);
	}

	std::vector<MeshMaterial> MaterialProcessor::LoadMTLMaterials(const std::vector<String>& paths)
	{
		std::vector<MeshMaterial> materials;

		for (const String& path : paths)
		{
			TextFileReader fileReader;
			fileReader.Read(path);

			String line = fileReader.NextLine();
			while (line != TextFileReader::END_OF_FILE_STRING)
			{
				if (line.StartsWith("newmtl"))
				{
					MeshMaterial material;
					line = line.SubStr(line.FindFirstOf(' ') + 1);
					material.name = line;

					line = fileReader.NextLine();
					if (line.StartsWith("Kd"))
					{
						line = line.SubStr(line.FindFirstOf(' ') + 1);
						std::vector<String> values = SplitString(line, ' ');
						real32 x = values[0].ToReal32();
						real32 y = values[1].ToReal32();
						real32 z = values[2].ToReal32();

						material.colourKd = Vec3f(x, y, z);
					}

					bool add = true;
					for (const MeshMaterial& mat : materials)
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