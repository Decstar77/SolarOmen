#pragma once
#include "SolarDefines.h"


#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for release builds.
#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif
namespace sol
{
	enum class LogLevel
	{
		FATAL = 0,
		ERROR = 1,
		WARN = 2,
		INFO = 3,
		DEBUG = 4,
		TRACE = 5
	};

	bool8 InitializeLogging();
	void ShutdownLogging();

	SOL_API void LogOutput(LogLevel level, const char* message);
}

// Logs a fatal-level message.
#define SOLFATAL(message) sol::LogOutput(sol::LogLevel::FATAL, message);

#ifndef SOLERROR
// Logs an error-level message.
#define SOLERROR(message) sol::LogOutput(sol::LogLevel::ERROR, message);
#endif

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message.
#define SOLWARN(message) sol::LogOutput(sol::LogLevel::WARN, message);
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define SOLWARN(message)
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message.
#define SOLINFO(message) sol::LogOutput(sol::LogLevel::INFO, message);
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define SOLINFO(message)
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a debug-level message.
#define SOLDEBUG(message) sol::LogOutput(sol::LogLevel::DEBUG, message);
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define SOLDEBUG(message)
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a trace-level message.
#define SOLTRACE(message) sol::LogOutput(sol::LogLevel::INFO, message);
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define SOLTRACE(message)
#endif
