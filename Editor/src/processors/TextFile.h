#pragma once
#include "../Core.h"

namespace sol
{
	class TextFileReader
	{
	public:
		inline static String END_OF_FILE_STRING = "__END_OF_FILE__";

		TextFileReader()
		{
			cursor = 0;
		}

		String NextLine();
		void Read(const String& path);

	private:
		uint32 cursor;
		std::vector<String> lines;
	};

	class TextFileWriter
	{
	public:
		TextFileWriter() {};
		inline void WriteLine(String line) { lines.push_back(line.Add('\n')); }
		void SaveToDisk(const String& path);
	private:
		std::vector<String> lines;
	};

}