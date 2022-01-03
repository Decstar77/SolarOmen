#include "SolarLogging.h"

#include <string>
#include <stdarg.h>

#include <iostream>

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
		std::cout << levelStrings[(uint32)level] << message << std::endl;
	}
}