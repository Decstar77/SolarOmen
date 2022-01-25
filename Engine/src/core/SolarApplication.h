#pragma once
#include "../SolarDefines.h"
#include "SolarString.h"

namespace sol
{
	struct SOL_API ApplicationConfigs
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


		static real32 GetDeltaTime();
		static uint32 GetSurfaceWidth();
		static uint32 GetSurfaceHeight();
		static real32 GetSurfaceAspectRatio();
	private:
		inline static real32 dt = 0.0f;
	};
}