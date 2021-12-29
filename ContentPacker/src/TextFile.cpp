#include "TextFile.h"

#include <fstream>

namespace cm
{
	CString TextFileReader::NextLine()
	{
		if (cursor == (uint32)lines.size())
			return END_OF_FILE_STRING;

		return lines.at(cursor++);
	}

	void TextFileReader::Read(const CString& path)
	{
		std::ifstream file(path.GetCStr());
		std::string line;
		while (std::getline(file, line)) {
			lines.push_back(line.c_str());
		}

		file.close();
	}
}