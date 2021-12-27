#pragma once
#include "Core.h"

namespace cm
{
	class TextFileReader
	{
	public:
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