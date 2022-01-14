#include "SolarLogging.h"
#include "platform/SolarPlatform.h"
#include <string>

namespace sol
{
	bool8 sol::InitializeLogging()
	{
		return 1;
	}

	void ShutdownLogging()
	{

	}

	void LogOutput(LogLevel level, const char* message)
	{
		const char* levelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };
		const char* header = levelStrings[(uint32)level];

		uint32 length = (uint32)strlen(message);
		char* msg = GameMemory::PushTransientCount<char>(length + 12);

		strcat_s(msg, length + 12, header);
		strcat_s(msg, length + 12, message);
		strcat_s(msg, length + 12, "\n");

		Platform::ConsoleWrite(msg, (uint8)level);

		if (level == LogLevel::FATAL)
		{
			Platform::DisplayError(message);
		}
	}
}