#pragma once
#include "../SolarDefines.h"
#include "../core/SolarMemory.h"
#include "../core/SolarString.h"
#include "../core/SolarContainers.h"

namespace sol
{
	struct SOL_API PlatformFile
	{
		String path;
		uint64 creationTime;
		uint64 lastAcessTime;
		uint64 lastWriteTime;
		uint64 sizeBytes;
		void* data;
	};

	struct SOL_API PlatformNetworkAddress
	{
		uint16 port;
		uint32 ipAddress;
		String stringIP;
	};

	class SOL_API Platform
	{
	public:
		// @NOTE: These memory functions must work before and after platform intialization.
		static void* AllocateMemory(uint64 size);
		static void FreeMemory(void* block);
		static void* ZeroMemory(void* block, uint64 size);
		static void* CopyMemory(void* dest, const void* source, uint64 size);
		static void* SetMemory(void* dest, int32 value, uint64 size);

		static bool8 Intialize(int32 x, int32 y, int32 width, int32 height);
		static void Shutdown();
		static bool8 PumpMessages();
		static void* GetNativeState();

		static bool8 WriteFile(const String& path, void* data, uint32 sizeBytes);
		static ManagedArray<String> LoadEntireFolder(const String& path, const String& fileTypes);
		static PlatformFile LoadEntireFile(const String& path, bool32 metaDataOnly);

		static String OpenNativeFileDialog();

		static void Quit();
		static uint32 GetWindowWidth();
		static uint32 GetWindowHeight();
		static uint32 GetSurfaceWidth();
		static uint32 GetSurfaceHeight();
		static void SetWindowPosition(int32 x, int32 y);

		static void DisplayError(const char* message);
		static void ConsoleWrite(const char* mssage, uint8 colour);

		static real64 GetAbsoluteTime();
		static void SleepThread(uint32 ms);

		static constexpr uint32 MAX_NETWORK_PACKET_SIZE = 256;
		static PlatformNetworkAddress NetworkStart(uint16 port);
		static int32 NetworkReceive(void* buf, int32 bufSizeBytes, PlatformNetworkAddress* address);
		static void NetworkSend(void* buf, int32 bufSizeBytes, const PlatformNetworkAddress& address);
		static void NetworkSend(void* buf, int32 bufSizeBytes, const String& address, uint16 port);
	};
}
