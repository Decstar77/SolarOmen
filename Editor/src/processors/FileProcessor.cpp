#include "FileProcessor.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace sol
{
	static std::vector<String> GetAllFilePaths(const String& path, const String& fileTypes)
	{
		HANDLE file = {};
		WIN32_FIND_DATAA fdFile = {};

		std::vector<String> paths;

		String searchString = {};
		searchString.Add(path).Add('*');
		if ((file = FindFirstFileA(searchString.GetCStr(), &fdFile)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (strcmp(fdFile.cFileName, ".") != 0
					&& strcmp(fdFile.cFileName, "..") != 0)
				{
					String filePath = String(path).Add(String(fdFile.cFileName));
					// @NOTE: Is the entity a File or Folder? 
					if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						auto moreFiles = GetAllFilePaths(filePath.Add('/'), fileTypes);
						for (auto f : moreFiles)
						{
							paths.push_back(f);
						}
					}
					else
					{
						String ext = Util::GetFileExtension(filePath);
						if (ext == fileTypes)
						{
							paths.push_back(filePath);
						}
					}
				}
			} while (FindNextFileA(file, &fdFile) != 0);
		}


		if (file)
		{
			FindClose(file);
		}

		return paths;
	}

	FileProcessor::FileProcessor()
	{

	}

	std::vector<String> FileProcessor::GetFilePaths(const String& path, const String& types)
	{
		return GetAllFilePaths(path, types);
	}
}