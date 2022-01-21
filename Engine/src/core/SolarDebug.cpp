#include "SolarDebug.h"

namespace sol
{
	ProfilerClock::ProfilerClock(const char* name)
	{
		this->functionName = String(name);
		clock.Start();
	}

	ProfilerClock::~ProfilerClock()
	{
		clock.Update();
		SOLINFO(functionName.Add(" took: ").Add((real32)clock.elapsed * 1000.0f).Add(" ms").GetCStr());
	}
}