#include "FileProcessor.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace cm
{
	static std::vector<CString> GetAllFilePaths(const CString& path, const CString& fileTypes)
	{
		HANDLE file = {};
		WIN32_FIND_DATAA fdFile = {};

		std::vector<CString> paths;

		CString searchString = {};
		searchString.Add(path).Add('*');
		if ((file = FindFirstFileA(searchString.GetCStr(), &fdFile)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (strcmp(fdFile.cFileName, ".") != 0
					&& strcmp(fdFile.cFileName, "..") != 0)
				{
					CString filePath = CString(path).Add(CString(fdFile.cFileName));
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
						CString ext = Util::GetFileExtension(filePath);
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

	std::vector<CString> FileProcessor::GetFilePaths(const CString& path, const CString& types)
	{
		return GetAllFilePaths(path, types);
	}
}