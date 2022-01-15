#include "TextFile.h"

#include <sstream>
#include <fstream>

namespace sol
{
	String TextFileReader::NextLine()
	{
		if (cursor == (uint32)lines.size())
			return END_OF_FILE_STRING;

		return lines.at(cursor++);
	}

	void TextFileReader::Read(const String& path)
	{
		std::ifstream file(path.GetCStr());
		std::string line;
		while (std::getline(file, line)) {
			lines.push_back(line.c_str());
		}

		file.close();
	}

	void TextFileWriter::SaveToDisk(const String& path)
	{
		std::ofstream file(path.GetCStr());

		if (file)
		{
			for (const String& line : lines)
			{
				file << line.GetCStr();
			}
		}
		else
		{
			Assert(0, "TODO ERROR SaveToDisk");
		}

		file.close();
	}
}