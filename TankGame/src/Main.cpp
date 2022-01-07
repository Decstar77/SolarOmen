
#include <SolarEngine.h>
#include <src/SolarEntry.h>

namespace sol
{
	struct GameState
	{
		Room room;
	};

	static GameState* gameState = nullptr;

	static bool8 GameInitialze(Game* game)
	{
		Room* room = &gameState->room;

		room->Initliaze();
		Entity entity = room->CreateEntity("cube");
		entity.SetMaterial("TankBase", "");

		return true;
	}

	static bool8 GameUpdate(Game* game, RenderPacket* renderPacket, real32 dt)
	{
		Room* room = &gameState->room;

		Input* input = Input::Get();
		if (IsKeyJustDown(input, escape))
		{
			return false;
		}

		room->ContructRenderPacket(renderPacket);

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

		gameState = GameMemory::PushPermanentStruct<GameState>();

		return true;
	}
}
