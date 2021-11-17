#pragma once
#include "core/SolarCore.h"

#if _WIN32

#include "../platform/win32/Clock.h"
#include "../platform/win32/Threading.h"

#endif


namespace cm
{
#define PROFILE_FUNCTION() ProfileClock __PROFILE_CLOCK__(__func__)

	class PlatformState
	{
	public:
		int64 window;
		int64 console;
		bool32 rawInput;
		bool32 isFocused;
		int32 client_width;
		int32 client_height;
		real32 aspect;

		inline static PlatformState* Get()
		{
			return platformState;
		}

		inline static void Initailize(PlatformState* state)
		{
			platformState = state;
		}

	private:
		inline static PlatformState* platformState = nullptr;
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

	class PlatformNetwork
	{
	public:
		static void Initialize();
		static void Shutdown();
	};

	CString PlatformOpenNFileDialogAndReturnPath();
	int32 PlatformCompareFileTimes(uint64 fileTime1, uint64 fileTime2);

	PlatformFolder DEBUGLoadEnitreFolder(const CString& file, const CString& fileTypes, bool32 metaDataOnly);
	PlatformFile DEBUGLoadEntireFile(const CString& file, bool32 metaDataOnly);
	void DEBUGFreeFile(PlatformFile* file);
	void DEBUGFreeFolder(PlatformFolder* folder);
	bool32 DEBUGWriteFile(PlatformFile file, const CString& name);
}
