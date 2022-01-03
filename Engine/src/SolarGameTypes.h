#pragma once

#include "core/SolarApplication.h"
namespace sol
{
	struct Game
	{
		ApplicationConfigs appConfig;
		bool8(*Initialize)(Game* game);
		bool8(*Update)(Game* game, real32 dt);
		bool8(*Render)(Game* game, real32 dt);
		void (*OnResize)(Game* game, uint32 width, uint32 height);
		void* gameState;
	};

	extern bool8 CreateGame(Game* game);
}