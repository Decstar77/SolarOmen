
#include <SolarEngine.h>
#include <src/SolarEntry.h>

namespace sol
{
	struct PlayerState
	{
		Vec3f velocity;
		real32 drag;
		real32 mass;
	};

	struct GameState
	{
		PlayerState playerState;
		Entity hostPlayer;
		Entity peerPlayer;

		Vec3f cameraOffset;

		Room room;
	};

	static GameState* gameState = nullptr;

	static bool8 GameInitialze(Game* game)
	{
		Room* room = &gameState->room;

		room->Initliaze(nullptr);
		gameState->hostPlayer = room->CreateEntity("Sphere");
		gameState->hostPlayer.SetMaterial("Sphere", "");
		gameState->hostPlayer.SetLocalTransform(Transform(Vec3f(), Quatf(), Vec3f(0.5f)));
		gameState->cameraOffset = room->camera.transform.position - gameState->hostPlayer.GetLocalTransform().position;
		room->skyboxId = Resources::GetTextureResource("FS002_Day_Sunless")->id;

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
