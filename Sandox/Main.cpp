
#include <SolarEngine.h>
#include <src/SolarEntry.h>

namespace sol
{


	struct PlayerState
	{
		Vec3f velocity;
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

		for (int32 x = 0; x < 4; x++)
		{
			for (int32 z = 0; z < 4; z++)
			{
				Entity grid = room->CreateEntity("Grid");
				grid.SetMaterial("Plane", "Prototype_Green1");

				real32 xx = ((real32)x - 1.5f) * 2.0f;
				real32 zz = ((real32)z - 1.5f) * 2.0f;

				grid.SetLocalTransform(Transform(Vec3f(xx, 0.0f, zz), EulerToQuat(90.0f, 0.0f, 0.0f), Vec3f(1)));
			}
		}

		Entity cy = room->CreateEntity("Grid");
		cy.SetMaterial("Cylinder", "Prototype_Green1");
		cy.SetLocalTransform(Transform(Vec3f(0, 0.5f, 6.0f)));

		room->skyboxId = Resources::GetTextureResource("FS002_Day_Sunless")->id;
		room->camera.transform.position = Vec3f(0, 1.6f, 0);
		room->camera.transform.LookAtLH(Vec3f(0, 1.6f, 1.0f));

		return true;
	}

	static bool8 OperateCameraAsFreeLook(Camera* camera, real32 dtime)
	{
		Input* input = Input::Get();
		bool8 operating = false;
		if (input->mb2)
		{
			input->mouse_locked = true;

			Vec2f delta = input->mouseDelta;
			delta.y = -delta.y;

			real32 mouse_sensitivity = 0.1f;

			real32 nyaw = camera->yaw - delta.x * mouse_sensitivity;
			real32 npitch = camera->pitch + delta.y * mouse_sensitivity;

			npitch = Clamp(npitch, -89.0f, 89.0f);
			camera->yaw = Lerp(camera->yaw, nyaw, 0.67f);
			camera->pitch = Lerp(camera->pitch, npitch, 0.67f);

			Vec3f direction;
			direction.x = Cos(DegToRad(camera->yaw)) * Cos(DegToRad(camera->pitch));
			direction.y = Sin(DegToRad(camera->pitch));
			direction.z = Sin(DegToRad(camera->yaw)) * Cos(DegToRad(camera->pitch));

			Transform transform = camera->transform;

			Mat4f result = LookAtLH(transform.position, transform.position + direction, Vec3f(0, 1, 0));

			camera->transform.orientation = Mat4ToQuat(result);

			Basisf basis = camera->transform.GetBasis();

			real32 move_speed = 6.0f;
			if (input->w) { camera->transform.position += (basis.forward * move_speed * dtime); }
			if (input->s) { camera->transform.position += (-1.0f * basis.forward * move_speed * dtime); }
			if (input->a) { camera->transform.position += (-1.0f * basis.right * move_speed * dtime); }
			if (input->d) { camera->transform.position += (basis.right * move_speed * dtime); }
			if (input->q) { camera->transform.position += (Vec3f(0, 1, 0) * move_speed * dtime); }
			if (input->e) { camera->transform.position += (Vec3f(0, -1, 0) * move_speed * dtime); }

			operating = true;
		}
		else
		{
			input->mouse_locked = false;
		}

		return operating;
	}

	static void OperateFPSCamera(PlayerState* playerState, Camera* camera, real32 dt)
	{
		Input* input = Input::Get();
		input->mouse_locked = true;

		Basisf basis = camera->transform.GetBasis();

		basis.forward.y = 0.0f;
		basis.right.y = 0.0f;
		basis.upward = Vec3f(0, 1, 0);
		basis.forward = Normalize(basis.forward);
		basis.right = Normalize(basis.right);

		Vec3f acceleration = Vec3f(0);

		const real32 walkSpeed = 64.0f;
		const real32 airSpeed = 16.0f;
		const real32 groundResistance = 14.0f;
		const real32 airResistance = 2.0f;
		const real32 lookSpeed = 0.1f;

		real32 speed = walkSpeed;
		real32 resistance = groundResistance;

		if (input->w) { acceleration += (basis.forward * speed); }
		if (input->s) { acceleration += (-1.0f * basis.forward * speed); }
		if (input->a) { acceleration += (-1.0f * basis.right * speed); }
		if (input->d) { acceleration += (basis.right * speed); }

		acceleration = ClampMag(acceleration, speed);

		acceleration.x += -resistance * playerState->velocity.x;
		acceleration.z += -resistance * playerState->velocity.z;

		playerState->velocity = acceleration * dt + playerState->velocity;
		camera->transform.position += playerState->velocity * dt;



		// @NOTE: Look/aiming

		Vec2f delta = input->mouseDelta;
		delta.y = -delta.y;

		real32 nyaw = camera->yaw - delta.x * lookSpeed;
		real32 npitch = camera->pitch + delta.y * lookSpeed;

		npitch = Clamp(npitch, -89.0f, 89.0f);
		camera->yaw = Lerp(camera->yaw, nyaw, 0.67f);
		camera->pitch = Lerp(camera->pitch, npitch, 0.67f);

		Vec3f direction;
		direction.x = Cos(DegToRad(camera->yaw)) * Cos(DegToRad(camera->pitch));
		direction.y = Sin(DegToRad(camera->pitch));
		direction.z = Sin(DegToRad(camera->yaw)) * Cos(DegToRad(camera->pitch));

		Transform transform = camera->transform;
		Mat4f result = LookAtLH(transform.position, transform.position + direction, Vec3f(0, 1, 0));
		camera->transform.orientation = Mat4ToQuat(result);

		// Vec3f gravity = Vec3f(0.f, -15.8f, 0.f);
		//Vec3f currentPos = camera->transform.position;
		//playerEntity->transform.position += playerPart->velocity * gs->dt;
	}

	static bool8 GameUpdate(Game* game, RenderPacket* renderPacket, real32 dt)
	{
		Room* room = &gameState->room;

		Input* input = Input::Get();
		if (IsKeyJustDown(input, escape))
		{
			return false;
		}

		if (IsKeyJustDown(input, f2))
		{
			gameState = GameMemory::PushPermanentStruct<GameState>();
			return GameInitialze(game);
		}


		OperateFPSCamera(&gameState->playerState, &room->camera, dt);

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
