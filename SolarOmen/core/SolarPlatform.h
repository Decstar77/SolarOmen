#pragma once
#include "core/SolarCore.h"

#if _WIN32

#include "../platform/win32/Clock.h"
#include "../platform/win32/Threading.h"

#endif


namespace cm
{
#define PROFILE_FUNCTION() ProfileClock __PROFILE_CLOCK__(__func__)
#define GetPlatofrmState() PlatformState *ps = PlatformState::Get()
	class PlatformState
	{
	public:
		int64 window;
		int64 console;
		bool32 rawInput;
		bool32 isFocused;
		int32 clientWidth;
		int32 clientHeight;
		real32 aspect;

		inline static PlatformState* Get()
		{
			return platformState;
		}

		inline static void Initialize(PlatformState* ps)
		{
			platformState = ps;
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
		uint64 sizeBytes;
		void* data;
	};


	enum class SnapShotType : uint8
	{
		// NOTE: Messages 0 and 1 are resvered for the platform layer
		//INVALID = 0
		//CONNECTION = 1 
		TRANSFORM = 2,
	};

	struct SnapShotTransform
	{
		SnapShotType type;
		Vec3f position;
		Quatf orientation;
	};

	namespace Platform
	{
		bool32 Initialize(PlatformState* ps, const char* title, int32 width, int32 height, bool32 console);
		bool32 ProcessInput(PlatformState* ps, Input* input);
		void PostQuitMessage();
		void Shutdown();

		void IntializeThreads();
		void ShutdownThreads();

		void IntializeNetworking();
		void NetworkStart();
		void NetworkStart(const CString& ip);
		bool32 NetworkConnectionEsablished();
		int32 NetworkReceive(void* buf, int32 bufSizeBytes);
		void NetworkSend(void* buf, int32 bufSizeBytes);
		void ShutdownNetworking();

		bool32 WriteFile(const CString& path, void* data, uint32 sizeBytes);
		ManagedArray<CString> LoadEntireFolder(const CString& path, const CString& fileTypes);
		PlatformFile LoadEntireFile(const CString& path, bool32 metaDataOnly);
	}

	//CString PlatformOpenNFileDialogAndReturnPath();
	//int32 PlatformCompareFileTimes(uint64 fileTime1, uint64 fileTime2);

	//PlatformFolder DEBUGLoadEnitreFolder(const CString& file, const CString& fileTypes, bool32 metaDataOnly) { return {}; };
	//PlatformFile DEBUGLoadEntireFile(const CString& file, bool32 metaDataOnly);
	//void DEBUGFreeFile(PlatformFile* file);
	//void DEBUGFreeFolder(PlatformFolder* folder) {};
	//bool32 DEBUGWriteFile(PlatformFile file, const CString& name);
}
