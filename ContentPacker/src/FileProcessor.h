#pragma once

#include "Core.h"
namespace cm
{
	class FileProcessor
	{
	public:
		FileProcessor();
		std::vector<CString> GetFilePaths(const CString& path, const CString& types);

		DISABLE_COPY_AND_MOVE(FileProcessor);
	};


}