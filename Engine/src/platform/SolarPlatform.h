#pragma once
#include <SolarDefines.h>

namespace sol
{
	class Platform
	{
	public:
		static bool8 Intialize(int32 x, int32 y, int32 width, int32 height);
		static void Shutdown();
		static bool8 PumpMessages();

		static void* Allocate(uint64 size);
		static void Free(void* block);
		static void* ZeroMemory(void* block, uint64 size);
		static void* CopyMemory(void* dest, const void* source, uint64 size);
		static void* SetMemory(void* dest, int32 value, uint64 size);

		static void ConsoleWrite(const char* message, uint8 colour);

		static real64 GetAbsoluteTime();

		// Sleep on the thread for the provided ms. This blocks the main thread.
		// Should only be used for giving time back to the OS for unused update power.
		// Therefore it is not exported.
		static void Sleep(real64 ms);
	};
}
