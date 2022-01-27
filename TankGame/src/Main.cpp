
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

	static void UpdatePlayer(real32 dt)
	{
		real32 forwardForceStrength = 20.0f;
		real32 strafeForceStrength = 10.0f;
		real32 rocketImpuseStrength = 2.0f;
		real32 maxSpeed = 15.0f;

		Input* input = Input::Get();
		Room* room = &gameState->room;
		PlayerState* player = &gameState->playerState;

		player->drag = 0.97f;
		player->mass = 2.0f;

		real32 forwardDir = (real32)(input->w - input->s);
		real32 strafeDir = (real32)(input->d - input->a);

		Transform transform = gameState->hostPlayer.GetLocalTransform();
		Basis basis = transform.GetBasis();
		basis.forward.y = 0;
		basis.right.y = 0;
		basis.forward = Normalize(basis.forward);
		basis.right = Normalize(basis.right);

		Vec3f appliedAcceleration = Vec3f(0);
		appliedAcceleration += (basis.forward * forwardDir * forwardForceStrength) / player->mass;
		appliedAcceleration += (basis.right * strafeDir * strafeForceStrength) / player->mass;

		player->velocity += appliedAcceleration * dt;

		if (input->space)
		{
			player->velocity += basis.forward * rocketImpuseStrength;
		}

		if (MagSqrd(player->velocity) > maxSpeed * maxSpeed)
		{
			player->velocity = Normalize(player->velocity);
			player->velocity = player->velocity * maxSpeed;
		}

		transform.position += player->velocity * dt;
		transform.position.y = 0.25f;
		player->velocity = player->velocity * player->drag;

		Plane plane = Plane(Vec3f(0), Vec3f(0, 1, 0));
		Ray ray = room->camera.ShootRayAtMouse();
		RaycastInfo info = {};

		if (Raycast::CheckPlane(ray, plane, &info))
		{
			transform.LookAtLH(info.point);
		}


		transform.GlobalRotateX(-1.0f * DegToRad(7.0f) * forwardDir);
		transform.GlobalRotateZ(-1.0f * DegToRad(10.0f) * strafeDir);

		gameState->hostPlayer.SetLocalTransform(transform);
		Vec3f cameraPos = transform.position + gameState->cameraOffset;
		room->camera.transform.position = Lerp(room->camera.transform.position, cameraPos, 0.15f);
	}

	static bool8 GameUpdate(Game* game, RenderPacket* renderPacket, real32 dt)
	{
		Room* room = &gameState->room;

		Input* input = Input::Get();
		if (IsKeyJustDown(input, escape))
		{
			return false;
		}

		UpdatePlayer(dt);

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
