
#include <SolarEngine.h>
#include "ImGuiLayer.h"

namespace sol
{
	static EditorState* es = nullptr;
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
		UpdateImGui(es, dt);

		Input* input = Input::Get();
		if (IsKeyJustDown(input, escape))
		{
			return false;
		}

		return true;
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

		es = GameMemory::PushPermanentStruct<EditorState>();


		return true;
	}

}