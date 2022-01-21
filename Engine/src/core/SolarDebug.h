#pragma once
#include "../SolarDefines.h"
#include "SolarLogging.h"
#include "SolarString.h"
#include "SolarClock.h"

namespace sol
{
	class ProfilerClock
	{
	public:
		SOL_API  ProfilerClock(const char* name);
		SOL_API  ~ProfilerClock();
	private:
		String functionName;
		Clock clock;
	};

#define PROFILE_FUNCTION() ProfilerClock __PROFILE_CLOCK__(__func__)

}