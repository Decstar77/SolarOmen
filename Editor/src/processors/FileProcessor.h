#pragma once

#include "../Core.h"
namespace sol
{
	class FileProcessor
	{
	public:
		FileProcessor();
		std::vector<String> GetFilePaths(const String& path, const String& types);

		DISABLE_COPY_AND_MOVE(FileProcessor);
	};
}