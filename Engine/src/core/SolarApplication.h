#pragma once
#include "SolarDefines.h"
#include "SolarString.h"

namespace sol
{
	struct ApplicationConfigs
	{
		int16 startPosY;
		int16 startPosX;
		int16 startWidth;
		int16 startHeight;
		String name;
	};

	class SOL_API Application
	{
	public:
		static bool8 Initialize(struct Game* game);
		static bool8 Run(struct Game* game);
		static void Shutdown();
	};
}