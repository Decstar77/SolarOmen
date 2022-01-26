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

	bool8 TextFileReader::Read(const String& path)
	{
		std::ifstream file(path.GetCStr());
		if (file.is_open())
		{
			std::string line;
			while (std::getline(file, line)) {
				lines.push_back(line.c_str());
			}

			file.close();

			return true;
		}

		SOLERROR(String("Could not open: ").Add(path).GetCStr());

		return false;
	}

	bool8 TextFileWriter::SaveToDisk(const String& path)
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
			SOLERROR(String("Could not save: ").Add(path).GetCStr());
			return false;
		}

		file.close();

		return true;
	}
}