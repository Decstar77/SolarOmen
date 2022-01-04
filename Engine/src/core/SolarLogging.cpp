#include "SolarLogging.h"
#include "platform/SolarPlatform.h"

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
		String msg = levelStrings[(uint32)level];
		msg.Add(message).Add('\n');
		Platform::ConsoleWrite(msg, (uint8)level);
	}
}