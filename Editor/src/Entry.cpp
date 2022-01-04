
#include <SolarEngine.h>
#include "src/SolarEntry.h"

namespace sol
{
	struct GameState
	{

	};

	static bool8 GameInitialze(Game* game)
	{
		return 1;
	}

	static bool8 GameUpdate(Game* game, real32 dt)
	{
		Input* input = Input::Get();
		if (IsKeyJustDown(input, escape))
		{
			SOLINFO("Es");
		}

		return 1;
	}

	static bool8 GameRender(Game* game, real32 dt)
	{
		return 1;
	}

	static void GameOnResize(Game* game, uint32 width, uint32 height)
	{

	}

	bool8 CreateGame(Game* game)
	{
		game->appConfig.startPosX = 100;
		game->appConfig.startPosY = 100;
		game->appConfig.startWidth = 1280;
		game->appConfig.startHeight = 720;
		game->appConfig.name = "Engine Editor";
		game->Initialize = GameInitialze;
		game->Update = GameUpdate;
		game->Render = GameRender;
		game->OnResize = GameOnResize;

		game->gameState = GameMemory::PushPermanentStruct<GameState>();

		return true;
	}

}