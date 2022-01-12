#pragma once
#include "../SolarDefines.h"
#include "SolarLogging.h"
#include "SolarString.h"
#include "SolarClock.h"

namespace sol
{
	class SOL_API ProfilerClock
	{
	public:
		ProfilerClock(const char* name)
		{
			this->functionName = String(name);
			clock.Start();
		}

		~ProfilerClock()
		{
			clock.Update();
			SOLINFO(functionName.Add(" took: ").Add((real32)clock.elapsed * 1000.0f).Add(" ms").GetCStr());
		}

	private:
		String functionName;
		Clock clock;
	};

#define PROFILE_FUNCTION() ProfilerClock __PROFILE_CLOCK__(__func__)

}