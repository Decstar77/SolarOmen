#pragma once
#include "SolarDefines.h"

namespace sol
{
	class SOL_API Clock
	{
	public:
		real64 startTime;
		real64 elapsed;

		void Start();
		void Update();
		void Stop();
	};
}