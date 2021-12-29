#pragma once
#include "Core.h"

namespace cm
{
	class TextFileReader
	{
	public:
		inline static CString END_OF_FILE_STRING = "__END_OF_FILE__";

		TextFileReader()
		{
			cursor = 0;
		}

		CString NextLine();
		void Read(const CString& path);

	private:
		uint32 cursor;
		std::vector<CString> lines;
	};

}