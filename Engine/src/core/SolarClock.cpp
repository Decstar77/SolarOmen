#include "SolarClock.h"

#include "platform/SolarPlatform.h"

namespace sol
{
	void Clock::Start()
	{
		startTime = Platform::GetAbsoluteTime();
		elapsed = 0;
	}

	void Clock::Update()
	{
		if (startTime != 0)
		{
			elapsed = Platform::GetAbsoluteTime() - startTime;
		}
	}

	void Clock::Stop()
	{
		startTime = 0;
	}
}
