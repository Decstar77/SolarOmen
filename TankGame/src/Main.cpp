
#include <SolarEngine.h>
#include <src/SolarEntry.h>

namespace sol
{
	struct GameState
	{
		Entity hostPlayer;
		Entity peerPlayer;

		Vec3f cameraOffset;

		Room room;
	};

	static GameState* gameState = nullptr;

	static bool8 GameInitialze(Game* game)
	{
		Room* room = &gameState->room;

		room->Initliaze();
		gameState->hostPlayer = room->CreateEntity("cube");
		gameState->hostPlayer.SetMaterial("alien", "");
		gameState->cameraOffset = room->camera.transform.position - gameState->hostPlayer.GetLocalTransform().position;

		Entity terrian = room->CreateEntity(String("terrain"));
		terrian.SetLocalTransform(Transform(Vec3f(), Quatf(), Vec3f(100)));
		terrian.SetMaterial("terrain", "");

		for (uint32 i = 0; i < RandomUInt<uint32>(5, 10); i++)
		{
			const char* hangars[] = {
				"hangar_smallA",
				"hangar_smallB",
				"hangar_largeA",
				"hangar_largeB",
				"hangar_roundGlass",
				"hangar_roundA",
				"hangar_roundB",
			};

			uint32 index = RandomUInt((uint32)0, (uint32)ArrayCount(hangars));

			Entity hangar = room->CreateEntity(String("hangar").Add(i));
			Vec3f spawnPoint = RandomPointOnUnitSphere() * RandomReal(1.0f, 90.0f);

			hangar.SetMaterial(hangars[index], "");
			hangar.SetLocalTransform(Transform(Vec3f(spawnPoint.x, 0.0f, spawnPoint.z)));
		}

		for (uint32 i = 0; i < RandomUInt<uint32>(5, 25); i++)
		{
			Vec3f spawnPoint = RandomPointOnUnitSphere() * RandomReal<real32>(1.0f, 90.0f);
			Entity turretBase = room->CreateEntity(String("TurretBase").Add(i));
			//Entity turretGun = room->CreateEntity(String("TurretGun").Add(i));

			turretBase.SetMaterial("turret_double", "");
			//turretGun.SetMaterial("turret", "");

			Transform  t = Transform();
			t.position = spawnPoint;
			t.position.y = 0.0f;

			turretBase.SetLocalTransform(t);
			//turretGun.SetLocalTransform(t);
		}

		return true;
	}

	static void UpdatePlayer(real32 dt)
	{
		Input* input = Input::Get();
		Room* room = &gameState->room;

		Vec3f dir = Vec3f(0);
		dir.x = (real32)(input->d - input->a);
		dir.z = (real32)(input->w - input->s);
		dir = Normalize(dir);
		dir = dir * 3.0f * dt;

		Transform transform = gameState->hostPlayer.GetLocalTransform();
		transform.position += dir;

		Plane plane = Plane::Create(Vec3f(0), Vec3f(0, 1, 0));
		Ray ray = room->camera.ShootRayAtMouse();
		RaycastInfo info = {};

		if (Raycast::CheckPlane(ray, plane, &info))
		{
			transform.LookAtLH(info.closePoint);
		}

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
