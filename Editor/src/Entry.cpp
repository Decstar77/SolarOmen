
#include <SolarEngine.h>
#include "ImGuiLayer.h"

namespace sol
{
	struct GameState
	{

	};


	static bool8 GameInitialze(Game* game)
	{
		if (InitialzieImGui())
		{
			return true;
		}

		return false;
	}

	static bool8 GameUpdate(Game* game, real32 dt)
	{
		UpdateImGui();
		return true;
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