#pragma once
#include "core/SolarCore.h"
#include "../SimpleColliders.h"
#include "../ManifoldTests.h"

#if _WIN32

#include "win32/Clock.h"
#include "win32/Threading.h"
#include "win32/Network.h"

#endif


namespace cm
{
	// @NOTE: Going to use some c++ destructor crazyy stuff here
#define PROFILE_FUNCTION() ProfileClock __PROFILE_CLOCK__(__func__)

	struct PlatformState
	{
		int64 window;
		int64 console;
		bool32 rawInput;
		bool32 isFocused;
		int32 client_width;
		int32 client_height;
		real32 aspect;
	};

	struct PlatformFile
	{
		CString path;
		uint64 creationTime;
		uint64 lastAcessTime;
		uint64 lastWriteTime;
		uint64 size_bytes;
		void* data;
	};

	struct PlatformFolder
	{
		CString path;
		std::vector<PlatformFile> files;
	};

	CString PlatformOpenNFileDialogAndReturnPath();

	int32 PlatformCompareFileTimes(uint64 fileTime1, uint64 fileTime2);

	PlatformFolder DEBUGLoadEnitreFolder(const CString& file, const CString& fileTypes, bool32 metaDataOnly);

	PlatformFile DEBUGLoadEntireFile(const CString& file, bool32 metaDataOnly);

	void DEBUGFreeFile(PlatformFile* file);

	void DEBUGFreeFolder(PlatformFolder* folder);

	bool32 DEBUGWriteFile(PlatformFile file, const CString& name);
}
