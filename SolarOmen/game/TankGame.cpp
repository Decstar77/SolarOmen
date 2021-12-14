#include "TankGame.h"
#include "core/SolarCore.h"
#include "core/SolarGame.h"


namespace cm
{
	Entity Room::CreateEntity()
	{
		EntityId id = GetNextFreeEntityId();
		Assert(id.index > 0 && id.index < (int32)entities.GetCapcity(), "Entity id index was invalid");

		Entity* entity = &entities[id.index];
		Assert(entity->id.index == INVALID_ENTITY_INDEX, "Using a valid entity");
		entity->id = id;
		entity->room = this;

		transformComponents[id.index].transform = Transform();

		return *entity;
	}

	Entity Room::CreateEntity(const CString& name)
	{
		Entity entity = CreateEntity();
		entity.SetName(name);

		return entity;
	}

	void Room::DestoryEntity(Entity entity)
	{
		if (entity.IsValid())
		{
			PushFreeEntityId(entity.id);

			int32 index = entity.id.index;
			ZeroStruct(&entities[index]);
			ZeroStruct(&nameComponents[index]);
			ZeroStruct(&renderComponents[index]);
			ZeroStruct(&colliderComponents[index]);

			// @NOTE: We don't zero the brain beacause if the brain is calling this function it will
			//		: mess the current state of the brain, and thus any futher code is be broken entirely
			//		: instead what happens is the brain is zero'd upon being set.
			//ZeroStruct(&brainComponents[index]);
		}
	}

	void Room::BeginEntityLoop()
	{
		entityLoopIndex = 0;
	}

	Entity Room::GetNextEntity()
	{
		entityLoopIndex++;

		while (entityLoopIndex < entities.GetCapcity())
		{
			Entity entity = entities[entityLoopIndex];
			if (entity)
			{
				return entity;
			}

			entityLoopIndex++;
		}

		return {};
	}

	RaycastResult Room::ShootRayThrough(const Ray& ray)
	{
		real32 minDist = REAL_MAX;
		RaycastResult result = {};

		BeginEntityLoop();
		while (Entity entity = GetNextEntity())
		{
			int32 index = entity.id.index;
			ColliderComponent* cc = &colliderComponents[index];
			TransformComponent* tr = &transformComponents[index];

			if (cc->enabled)
			{
				bool hit = false;
				RaycastInfo info = {};

				switch (cc->type)
				{
				case ColliderType::SPHERE:
				{
					Sphere sphere = TranslateSphere(cc->sphere, tr->transform.position);
					hit = RaycastSphere(ray, sphere, &info);
				}
				break;
				case ColliderType::ALIGNED_BOUNDING_BOX:
				{
					AABB box = UpdateAABB(cc->alignedBox, tr->transform.position, tr->transform.orientation, tr->transform.scale);
					hit = RaycastAABB(ray, box, &info);
				}
				break;
				case ColliderType::BOUDNING_BOX:
				{
				}
				break;
				}

				if (hit && info.t < minDist)
				{
					minDist = info.t;
					result.hit = true;
					result.entity = entity;
					result.rayInfo = info;
				}
			}
		}

		return result;
	}

	void Room::Initialize()
	{
		bool twoPlayerGame = true;

		GetPlatofrmState();
		CreateEntityFreeList();

		Entity tank = CreateEntity("Tank");
		tank.SetModel("TankBase");
		tank.SetTexture("Tank_DefaultMaterial_BaseColor");
		tank.SetLocalTransform(Transform(Vec3f(0, 0.1f, 0), Quatf(), Vec3f(0.65f)));
		tank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));

		Entity turret = CreateEntity("Turret");
		turret.EnableRendering();
		turret.SetModel("TankTurret");
		turret.SetTexture("Tank_DefaultMaterial_BaseColor");
		turret.SetLocalTransform(Transform(Vec3f(0, 1.1f, 0), EulerToQuat(Vec3f(-8, 0, 0)), Vec3f(0.65f)));

		multiplayerState.player1Tank = tank;
		multiplayerState.player1Turret = turret;
		multiplayerState.sendTick = true;
		{
			Entity bulletSpawnPoint = CreateEntity("BulletSpawnPoint");
			bulletSpawnPoint.SetLocalTransform(Transform(Vec3f(0, 0.6f, 1.3f)));
			bulletSpawnPoint.SetCollider(CreateSphere(Vec3f(0, 0, 0), 0.1f));
			bulletSpawnPoint.SetParent(turret);

			BrainComponent* brain = tank.SetBrain(BrainType::Value::PLAYER_BRAIN);
			brain->playerBrain.tank = tank;
			brain->playerBrain.turret = turret;
			brain->playerBrain.bulletSpawnPoint = bulletSpawnPoint;

			playerCamera = {};
			playerCamera.far_ = 100.0f;
			playerCamera.near_ = 0.3f;
			playerCamera.yfov = 45.0f;
			playerCamera.aspect = (real32)ps->clientWidth / (real32)ps->clientHeight;
			playerCamera.transform = Transform();
			playerCamera.transform.position.x = 7;
			playerCamera.transform.position.y = 14;
			playerCamera.transform.position.z = -7;
			playerCamera.transform.LookAtLH(0);

			playerCameraOffset = playerCamera.transform.position - tank.GetWorldTransform().position;
		}

		if (twoPlayerGame)
		{
			Entity player2Tank = CreateEntity("Tank2");
			player2Tank.SetModel("TankBase");
			player2Tank.SetTexture("Tank_DefaultMaterial_BaseColor");
			player2Tank.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(0.65f)));
			player2Tank.SetCollider(CreateSphere(Vec3f(0, 0, 0), 1.0f));

			Entity player2Turret = CreateEntity("Turret");
			player2Turret.EnableRendering();
			player2Turret.SetModel("TankTurret");
			player2Turret.SetTexture("Tank_DefaultMaterial_BaseColor");
			player2Turret.SetLocalTransform(Transform(Vec3f(0, 1.1f, 0), EulerToQuat(Vec3f(0, 0, -4)), Vec3f(0.65f)));

			BrainComponent* brain = player2Tank.SetBrain(BrainType::Value::PEER_BRAIN);
			brain->networkBrain.player2Tank = player2Tank;
			brain->networkBrain.player2Turret = player2Turret;
			brain->networkBrain.player1Tank = tank;
			brain->networkBrain.player1Turret = turret;
		}

		if (0)
		{
			Entity tankAI = CreateEntity("TankAI");
			tankAI.SetRendering("TankBase", "Tank_DefaultMaterial_BaseColor");
			tankAI.SetLocalTransform(Transform(Vec3f(7, 0.1f, 7), EulerToQuat(Vec3f(0, 180, 0)), Vec3f(0.65f)));
			tankAI.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));

			Entity turretAI = CreateEntity("Turret");
			turretAI.SetRendering("TankTurret", "Tank_DefaultMaterial_BaseColor");
			turretAI.SetLocalTransform(Transform(Vec3f(7, 0.75f, 7), Quatf(), Vec3f(0.65f)));

			Entity bulletSpawnPoint = CreateEntity("BulletSpawnPoint");
			bulletSpawnPoint.SetLocalTransform(Transform(Vec3f(0, 0.6f, 1.3f)));
			bulletSpawnPoint.SetCollider(CreateSphere(Vec3f(0, 0, 0), 0.1f));
			bulletSpawnPoint.SetParent(turretAI);

			BrainComponent* brain = turretAI.SetBrain(BrainType::Value::TANK_AI_IMMOBILE);
			brain->tankAIImmobile.tank = tankAI;
			brain->tankAIImmobile.player1Tank = tank;
			brain->tankAIImmobile.bulletSpawnPoint = bulletSpawnPoint;
		}

		Entity prop = CreateEntity("Prop01");
		prop.EnableRendering();
		prop.SetModel("Prop_01");
		prop.SetTexture("Prop_01_DefaultMaterial_BaseColor");
		prop.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(1)));


		grid.Initialize();

		int32 map[] =
		{
			1,1,1,1,1,1,1,1,1,1,
			1,0,0,0,0,0,0,0,0,1,
			1,0,0,0,1,1,0,0,0,1,
			1,0,0,0,1,1,0,0,0,1,
			1,1,0,0,0,0,0,0,1,1,
			1,1,0,0,0,0,0,0,1,1,
			1,0,0,0,1,1,0,0,0,1,
			1,0,0,0,1,1,0,0,0,1,
			1,0,0,0,0,0,0,0,0,1,
			1,1,1,1,1,1,1,1,1,1,
		};

		Entity gridEntity = CreateEntity("Grid");

		for (int32 index = 0; index < ArrayCount(map); index++)
		{
			GridCell* cell = &grid.cells[index];
			int32 occupied = map[index];
			cell->occupied = occupied;
			if (occupied == 0)
			{
				Entity entity = CreateEntity("EmptyCell");
				entity.SetLocalTransform(Transform(Vec3f(cell->position.x, 0.0f, cell->position.y)));
				entity.SetModel("20x20_Floor");
				entity.SetTexture("Set0_Texture");
				entity.SetParent(gridEntity);
				cell->entity = entity;
			}
			else if (occupied == 1)
			{
				Entity entity = CreateEntity("Cell");
				entity.SetLocalTransform(Transform(Vec3f(cell->position.x, 0.0f, cell->position.y)));
				entity.SetModel("20x20_Full");
				entity.SetTexture("Set0_Texture");
				entity.EnableCollider();
				entity.SetCollider(CreateAABBFromCenterRadius(Vec3f(0, 1, 0), Vec3f(1)));
				entity.SetParent(gridEntity);
				cell->entity = entity;
			}
		}

		// @HACK: Just doing this because of editor restartign !!
		static bool isPlaying = false;
		if (!isPlaying)
		{
			//PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/Monsters.wav", true);
			isPlaying = true;
		}

	}

	static void SpawnBullet(Room* room, Transform transform)
	{
		transform.scale = Vec3f(0.2f);
		Basisf basis = transform.GetBasis();
		basis.forward.y = 0.0f;
		basis.forward = Normalize(basis.forward);
		basis.upward = Vec3f(0, 1, 0);
		basis.right = Cross(basis.upward, basis.forward);
		transform.orientation = Mat3ToQuat(basis.mat);

		Entity entity = room->CreateEntity("Bullet");
		entity.SetCollider(CreateSphere(Vec3f(0), 0.2f));
		entity.SetLocalTransform(transform);
		entity.SetModel("sphere");

		room->bullets.Add(entity);

		BrainComponent* bc = entity.SetBrain(BrainType::Value::BULLET);
		BulletBrain* bullet = &bc->bulletBrain;

		PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/gun_revolver_pistol_shot_04.wav", false);
	}

	GameUpdate* MultiplayerState::GetLatestValidGameUpdate()
	{
		int32 minIndex = -1;
		int32 minTick = INT32_MAX;
		for (int32 i = 0; i < (int32)gameUpdates.count; i++)
		{
			GameUpdate* gameUpdate = &gameUpdates[i];
			if (gameUpdate->hostTick != 0 && gameUpdate->hostTick < minTick)
			{
				minIndex = i;
				minTick = gameUpdate->hostTick;
			}
			if (gameUpdate->peerTick != 0 && gameUpdate->peerTick < minTick)
			{
				minIndex = i;
				minTick = gameUpdate->peerTick;
			}
		}

		if (minIndex >= 0)
		{
			GameUpdate* gameUpdate = &gameUpdates[minIndex];
			if (gameUpdate->hostTick == gameUpdate->peerTick)
			{
				return gameUpdate;
			}
		}


		return nullptr;
	}

	GameUpdate* MultiplayerState::GetGameUpdate(int32 tickIndex)
	{
		for (uint32 i = 0; i < gameUpdates.count; i++)
		{
			GameUpdate* gameUpdate = &gameUpdates[i];
			if (gameUpdate->hostTick == tickIndex || gameUpdate->peerTick == tickIndex)
			{
				return gameUpdate;
			}
		}

		// @TODO: This might be too big for the stack !!
		GameUpdate update = {};
		return gameUpdates.Add(update);
	}

	void MultiplayerState::Update(Room* room, real32 dt)
	{
		GetInput();
		if (!startedNetworkStuff)
		{
			if (IsKeyJustDown(input, f9))
			{
				myAddress = Platform::NetworkStart(54000);
				startedNetworkStuff = true;
			}

			if (IsKeyJustDown(input, f10))
			{
				myAddress = Platform::NetworkStart(54001);
				startedNetworkStuff = true;
			}
		}

		if (startedNetworkStuff && IsKeyJustDown(input, f11))
		{
			SnapShot snapShot = {};
			snapShot.type = SnapShotType::HANDSHAKE_CONNECTION;
			int32 port = myAddress.port == 54000 ? 54001 : 54000;
			Platform::NetworkSend((void*)&snapShot, sizeof(SnapShot), "192.168.0.107", port);
		}

		uint8 buffer[Platform::MAX_NETWORK_PACKET_SIZE] = {};
		PlatformAddress address = {};
		while (Platform::NetworkReceive(buffer, sizeof(buffer), &address) > 0)
		{
			SnapShot* snap = (SnapShot*)buffer;
			if (snap->type == SnapShotType::HANDSHAKE_CONNECTION && !connectionValid)
			{
				Debug::LogInfo(CString("Connected to").Add((int32)address.port));
				peerAddress = address;
				Platform::NetworkSend(snap, sizeof(SnapShot), peerAddress);
				connectionValid = true;
			}
			if (snap->type == SnapShotType::PING)
			{
				if (!snap->snapPing.ack)
				{
					snap->snapPing.ack = true;
					Platform::NetworkSend(snap, sizeof(SnapShot), peerAddress);
				}
				else
				{
					Debug::LogInfo(CString("Ping: ").Add(pingTimer * 1000.0f));
				}
			}
			else if (snap->type == SnapShotType::TICK)
			{
				//Debug::LogInfo(CString("RecTick ").Add(snap->snapTick.tickNumber));
				unproccessedPeerTicks.Add(snap->snapTick);
			}
		}

		if (connectionValid)
		{
			timeSinceLastTick += dt;
			if ((timeSinceLastTick >= 1.0f / (real32)TICKS_PER_SECOND) && sendTick)
			{
				currentTick++;

				Transform t1 = player1Tank.GetWorldTransform();
				Transform t2 = player1Turret.GetWorldTransform();

				SnapShot snap = {};
				snap.type = SnapShotType::TICK;
				snap.snapTick.tickNumber = currentTick;
				snap.snapTick.playerSpawnBullet = spawnBullet;
				snap.snapTick.tankPosition = t1.position;
				snap.snapTick.tankOrientation = t1.orientation;
				snap.snapTick.turretPosition = t2.position;
				snap.snapTick.turretOrientation = t2.orientation;

				Platform::NetworkSend(&snap, sizeof(SnapShot), peerAddress);

				sendTick = false;
				timeSinceLastTick = 0.0f;

				GameUpdate* update = GetGameUpdate(currentTick);
				update->hostTick = currentTick;
				update->player1TankPos = snap.snapTick.tankPosition;
				update->player1TurretPos = snap.snapTick.turretPosition;
				update->player1TankOri = snap.snapTick.tankOrientation;
				update->player1TurretOri = snap.snapTick.turretOrientation;
				update->player1SpawnBullet = spawnBullet;

				spawnBullet = false;
				//Debug::LogInfo(CString("SndTick ").Add(currentTick));
			}
		}

		if (unproccessedPeerTicks.count > 0)
		{
			while (unproccessedPeerTicks.count > 0)
			{
				SnapGameTick snap = unproccessedPeerTicks[0];

				GameUpdate* update = GetGameUpdate(snap.tickNumber);
				update->peerTick = snap.tickNumber;
				update->player2TankPos = snap.tankPosition;
				update->player2TurretPos = snap.turretPosition;
				update->player2TankOri = snap.tankOrientation;
				update->player2TurretOri = snap.turretOrientation;
				update->player2SpawnBullet = snap.playerSpawnBullet;

				unproccessedPeerTicks.Remove((uint32)0);
			}
		}

		GameUpdate* gameUpdate = GetLatestValidGameUpdate();
		if (gameUpdate)
		{
			LOG(gameUpdate->hostTick << ":" << gameUpdate->peerTick);

			gameUpdates.Remove(gameUpdate);

			for (uint32 i = 0; i < room->brainComponents.GetCapcity(); i++)
			{
				BrainComponent* bc = &room->brainComponents[i];
				if (bc->enabled)
				{
					if (room->entities[i].IsValid())
					{
						switch (bc->type.Get())
						{
						case BrainType::Value::PLAYER_BRAIN:bc->playerBrain.TickUpdate(room, gameUpdate, dt); break;
						case BrainType::Value::BULLET:break;
						case BrainType::Value::PEER_BRAIN: bc->networkBrain.TickUpdate(room, gameUpdate, dt); break;
						case BrainType::Value::TANK_AI_IMMOBILE:break;
						}
					}
				}
			}

			if (gameUpdate->player1SpawnBullet)
			{
				Transform transform = {};
				transform.position = gameUpdate->player1TurretPos;
				transform.orientation = gameUpdate->player1TurretOri;
				//LOG("NET SPAWN" << lastTick.tickNumber);
				SpawnBullet(room, transform);
			}

			if (gameUpdate->player2SpawnBullet)
			{
				Transform transform = {};
				transform.position = gameUpdate->player2TurretPos;
				transform.orientation = gameUpdate->player2TurretOri;
				//LOG("LOCAL SPAWN " << currentTick);
				SpawnBullet(room, transform);
			}

			sendTick = true;
		}

	}

	void Room::Update(real32 dt)
	{
		multiplayerState.Update(this, dt);

		for (uint32 i = 0; i < brainComponents.GetCapcity(); i++)
		{
			BrainComponent* bc = &brainComponents[i];
			if (bc->enabled)
			{
				if (entities[i].IsValid())
				{
					switch (bc->type.Get())
					{
					case BrainType::Value::PLAYER_BRAIN: bc->playerBrain.FrameUpdate(this, dt);  break;
					case BrainType::Value::BULLET: bc->bulletBrain.FrameUpdate(this, entities[i], dt);  break;
					case BrainType::Value::PEER_BRAIN: bc->networkBrain.FrameUpdate(this, entities[i], dt);  break;
					case BrainType::Value::TANK_AI_IMMOBILE: bc->tankAIImmobile.FrameUpdate(this, entities[i], dt);  break;
					}
				}
			}
		}

		DEBUGDrawAllColliders();
	}

	void Room::ConstructRenderGroup(EntityRenderGroup* renderGroup)
	{
		renderGroup->mainCamera = playerCamera;
		renderGroup->playerCamera = playerCamera;

		renderGroup->entries.count = 0;
		BeginEntityLoop();
		while (Entity entity = GetNextEntity())
		{
			RenderComponent* render = &renderComponents[entity.id.index];

			CString name = entity.GetName();

			RenderEntry entry = {};
			entry.transform = entity.GetWorldTransform();
			entry.modelId = render->modelId;
			entry.textureId = render->textureId;
			entry.shaderId = render->shaderId;

			renderGroup->entries.Add(entry);
		}
	}

	void Room::Shutdown()
	{

	}

	void Room::DEBUGDrawAllColliders()
	{
		BeginEntityLoop();
		while (Entity entity = GetNextEntity())
		{
			ColliderComponent cc = entity.GetColliderLocal();
			Transform tr = entity.GetWorldTransform();

			if (cc.enabled)
			{
				switch (cc.type)
				{
				case ColliderType::SPHERE:
				{
					Sphere sphere = TranslateSphere(cc.sphere, tr.position);
					Debug::DrawSphere(sphere);
				}
				break;
				case ColliderType::ALIGNED_BOUNDING_BOX:
				{
					AABB box = UpdateAABB(cc.alignedBox, tr.position, tr.orientation, tr.scale);
					Debug::DrawAABB(box);
				}
				break;
				case ColliderType::BOUDNING_BOX:
				{
				}
				break;
				}
			}
		}
	}

	void Room::CreateEntityFreeList()
	{
		entityFreeList.count = ENTITY_STORAGE_COUNT - 1;
		for (int32 i = (int32)ENTITY_STORAGE_COUNT - 2; i >= 0; i--)
		{
			EntityId id = {};
			id.index = entityFreeList.count - i;
			id.generation = 0;
			entityFreeList[i] = id;
		}

		for (uint32 i = 0; i < entities.GetCapcity(); i++)
		{
			ZeroStruct(entities.Get(i));
		}
	}

	EntityId Room::GetNextFreeEntityId()
	{
		entityFreeList.count--;
		int32 index = entityFreeList.count;
		Assert(entityFreeList.count > 0, "No more free ids");
		EntityId* id = entityFreeList.Get(index);
		id->generation++;

		return *id;
	}

	void Room::PushFreeEntityId(EntityId id)
	{
		int32 index = entityFreeList.count;
		entityFreeList.count++;
		Assert(entityFreeList.count <= entityFreeList.GetCapcity(), "To many free ids");
		entityFreeList[index] = id;
	}

	bool32 Game::Initialize()
	{
		GameState::Initialize(GameMemory::PushPermanentStruct<GameState>());

		GetGameState();
		gs->currentRoom.Initialize();

		return true;
	}

	void Game::UpdateGame(real32 dt)
	{
		GetGameState();
		gs->currentRoom.Update(dt);
		//gs->currentRoom.grid.DebugDraw();
	}

	void Game::ConstructRenderGroup(EntityRenderGroup* renderGroup)
	{
		GetGameState();
		gs->currentRoom.ConstructRenderGroup(renderGroup);
	}

	void Game::Shutdown()
	{

	}


	void Grid::Initialize()
	{
		real32 hasCellExtent = CELL_EXTENT / 2.0f;
		real32 xOffet = (CELL_EXTENT * HORIZONTAL_CELL_COUNT) / 2.0f;
		real32 yOffet = (CELL_EXTENT * VERTICAL_CELL_COUNT) / 2.0f;

		center = Vec2f(0, 0);
		topLeft = center + Vec2f(-xOffet, -yOffet);
		topRight = center + Vec2f(xOffet, -yOffet);
		bottomRight = center + Vec2f(xOffet, yOffet);
		bottomLeft = center + Vec2f(-xOffet, yOffet);

		cells.count = HORIZONTAL_CELL_COUNT * VERTICAL_CELL_COUNT;
		for (int32 x = 0; x < HORIZONTAL_CELL_COUNT; x++)
		{
			for (int32 y = 0; y < VERTICAL_CELL_COUNT; y++)
			{
				int32 index = IndexOf2DArray(HORIZONTAL_CELL_COUNT, x, y);

				GridCell cell = {};
				cell.index = index;
				cell.xIndex = x;
				cell.yIndex = y;
				cell.occupied = false;
				cell.position = (Vec2f(CELL_EXTENT) * Vec2f((real32)x, (real32)y)) + Vec2f(hasCellExtent) + topLeft;

				cells[index] = cell;
			}
		}
	}

	void Grid::DebugDraw()
	{
		real32 h = 0.0f;

		for (int32 x = 0; x < HORIZONTAL_CELL_COUNT; x++)
		{
			Vec3f p1 = Vec3f(bottomLeft.x + x * CELL_EXTENT, h, bottomLeft.y);
			Vec3f p2 = Vec3f(topLeft.x + x * CELL_EXTENT, h, topLeft.y);
			Debug::DrawLine(p1, p2);
		}
		for (int32 y = 0; y < VERTICAL_CELL_COUNT; y++)
		{
			Vec3f p1 = Vec3f(topLeft.x, h, topLeft.y + y * CELL_EXTENT);
			Vec3f p2 = Vec3f(topRight.x, h, topRight.y + y * CELL_EXTENT);
			Debug::DrawLine(p1, p2);
		}

		for (uint32 i = 0; i < cells.GetCapcity(); i++)
		{
			Vec3f p1 = Vec3f(cells[i].position.x, h, cells[i].position.y);
			Vec3f p2 = p1 + Vec3f(0, 1, 0);
			Debug::DrawLine(p1, p2);
		}
	}

	static real32 NormalizeEulerRotation(real32 angle)
	{
		if (angle > 2 * PI)
		{
			angle -= 2 * PI;
		}
		if (angle < -2 * PI)
		{
			angle += 2 * PI;
		}

		return angle;
	}

	static real32 Cross2DScalar(const Vec2f a, const Vec2f b)
	{
		return (a.x * b.y) - (a.y * b.x);
	}

	void PlayerBrain::FrameUpdate(Room* room, real32 dt)
	{
		if (!initialized)
			Start(room, dt);

		UpdateBase(room, dt);
		UpdateTurret(room, dt);
		UpdateFiring(room, dt);

		//Sphere collider = tank.GetSphereColliderWorld();
		//for (uint32 i = 0; i < room->bullets.count; i++)
		//{
		//	Sphere sphere = room->bullets[i].GetSphereColliderWorld();
		//	if (CheckIntersectionSphere(sphere, collider))
		//	{
		//		PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/explosion_large_01.wav", false);
		//		room->DestoryEntity(tank);
		//		room->DestoryEntity(turret);
		//		break;
		//	}
		//}
	}

	void PlayerBrain::TickUpdate(Room* room, GameUpdate* update, real32 dt)
	{

	}

	void PlayerBrain::Start(Room* room, real32 dt)
	{
		turretRotation = 0.0f;
		tankRotation = turretRotation;
		initialized = true;
	}

	void PlayerBrain::UpdateTurret(Room* room, real32 dt)
	{
		Transform turretTransform = turret.GetLocalTransform();
		//turretTransform.LocalRotateY(DegToRad(25.5f * dt));

		Ray ray = room->playerCamera.ShootRayFromScreen();
		RaycastInfo rayInfo = {};
		if (RaycastPlane(ray, CreatePlane(Vec3f(0), Vec3f(0, 1, 0)), &rayInfo))
		{
			Vec2f point = Vec2f(rayInfo.closePoint.x, rayInfo.closePoint.z);
			Vec3f worldPos = turret.GetWorldTransform().position;

			Vec2f look = point - Vec2f(worldPos.x, worldPos.z);
			Vec2f dir = Vec2f(Sin(turretRotation), Cos(turretRotation));

			look = Normalize(look);
			dir = Normalize(dir);

			// @NOTE: 2D Cross product
			int32 sign = Sign(Cross2DScalar(look, dir));

			//Debug::DrawPoint(rayInfo.closePoint + Vec3f(0, worldPos.y, 0));
			//Debug::DrawLine(worldPos, worldPos + 100.0f * Vec3f(look.x, 0.0f, look.y));
			//Debug::DrawLine(worldPos, worldPos + 100.0f * Vec3f(dir.x, 0.0f, dir.y));

			real32 theta = Acos(Clamp(Dot(look, dir), 0.0f, 1.0f)) * sign;

			turretRotation = NormalizeEulerRotation(turretRotation + theta);

			turretTransform.LocalRotateY(theta);
		}

		turretTransform.position = tank.GetWorldTransform().position + Vec3f(0.0f, 0.65f, 0.0f);

		turret.SetLocalTransform(turretTransform);
	}

	void PlayerBrain::UpdateBase(Room* room, real32 dt)
	{
		GetInput();

		Transform tankTransform = tank.GetWorldTransform();

		int32 turnDir = input->d - input->a;
		int32 moveDir = input->w - input->s;

		real32 rotationDelta = turnDir * TANK_ROTATION_SPEED * dt;
		tankRotation += rotationDelta;
		tankTransform.LocalRotateY(rotationDelta);

		real32 moveDelta = moveDir * TANK_MOVE_SPEED * dt;
		Vec2f forwardDir = Vec2f(Sin(tankRotation), Cos(tankRotation));
		tankTransform.position += Vec3f(moveDelta * forwardDir.x, 0.0f, moveDelta * forwardDir.y);

		// @NOTE: Collision detection 
		Sphere tankCollider = tank.GetSphereColliderWorld();
		for (uint32 i = 0; i < room->grid.cells.count; i++)
		{
			GridCell* cell = &room->grid.cells[i];
			if (cell->occupied == 1)
			{
				AABB aabb = cell->entity.GetAlignedBoxColliderWorld();
				Manifold manifold = {};
				if (CheckManifoldSphereAABB(tankCollider, aabb, &manifold))
				{
					Vec3f disp = manifold.normal * manifold.seperationDistance;
					tankTransform.position += disp;

					tankCollider = TranslateSphere(tankCollider, disp);
				}

				//Debug::DrawAABB(aabb);
			}
		}

		Debug::DrawSphere(tankCollider);
		tank.SetLocalTransform(tankTransform);
		room->playerCamera.transform.position = Lerp(room->playerCamera.transform.position,
			tankTransform.position + room->playerCameraOffset, 0.1f);
	}

	static void DestroyBullet(Room* room, Entity entity)
	{
		int32 indexToRemove = -1;
		for (int32 i = 0; i < (int32)room->bullets.count; i++)
		{
			Assert(room->bullets[i].IsValid(), "");
			Assert(entity.IsValid(), "");

			if (room->bullets[i] == entity)
			{
				indexToRemove = i;
				room->DestoryEntity(entity);
				break;
			}
		}

		if (indexToRemove >= 0)
		{
			room->bullets.Remove(indexToRemove);
		}
	}

	void PlayerBrain::UpdateFiring(Room* room, real32 dt)
	{
		GetInput();
		lastFireTime += dt;

		bool canFireNow = lastFireTime > FIRE_RATE;
		if (canFireNow && !canFire)
		{
			canFire = true;
			PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/gun_shotgun_cock_01.wav", false);
		}

		if (IsKeyJustDown(input, mb2))
		{
			if (canFire)
			{
				lastFireTime = 0.0f;
				canFire = false;
				room->multiplayerState.spawnBullet = true;
			}
			else
			{
				PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/gun_rifle_dry_fire_01.wav", false);
			}
		}
	}

	void BulletBrain::FrameUpdate(Room* room, Entity entity, real32 dt)
	{
		Transform transform = entity.GetLocalTransform();
		Basisf basis = transform.GetBasis();

		Vec2f moveDir = Normalize(Vec2(basis.forward.x, basis.forward.z));
		Vec2f moveDelta = BULLET_MOVE_SPEED * moveDir * dt;

		transform.position += Vec3f(moveDelta.x, 0.0f, moveDelta.y);

		entity.SetLocalTransform(transform);

		// @NOTE: Collision detection 
		bool hitCollider = false;
		Sphere bulletCollider = entity.GetSphereColliderWorld();
		for (uint32 i = 0; i < room->grid.cells.count; i++)
		{
			GridCell* cell = &room->grid.cells[i];
			if (cell->occupied == 1)
			{
				AABB aabb = cell->entity.GetAlignedBoxColliderWorld();
				Manifold manifold = {};
				if (CheckManifoldSphereAABB(bulletCollider, aabb, &manifold))
				{
					Vec3f r = Reflect(Vec3f(moveDir.x, 0.0f, moveDir.y), manifold.normal);

					basis.forward = r;
					basis.upward = Vec3f(0, 1, 0);
					basis.right = Cross(basis.upward, basis.forward);
					transform.orientation = Mat3ToQuat(basis.mat);

					Vec3f disp = manifold.normal * manifold.seperationDistance;
					transform.position += disp;

					bulletCollider = TranslateSphere(bulletCollider, disp);
					hitCollider = true;
				}
			}
		}

		entity.SetLocalTransform(transform);

		if (hitCollider)
			collisionCount++;

		if (collisionCount == 2)
		{
			DestroyBullet(room, entity);
		}

	}

	void PeerBrain::TickUpdate(Room* room, GameUpdate* update, real32 dt)
	{
		player2TankLerpPos = update->player2TankPos;
		player2TankLerpOri = update->player2TankOri;

		player2TurretLerpPos = update->player2TurretPos;
		player2TurretLerpOri = update->player2TurretOri;
	}

	void PeerBrain::FrameUpdate(Room* room, Entity entity, real32 dt)
	{
		{
			Transform transform = player2Tank.GetWorldTransform();
			transform.position = Lerp(player2TankLerpPos, transform.position, 0.65f);
			transform.orientation = Slerp(player2TankLerpOri, transform.orientation, 0.65f);
			player2Tank.SetLocalTransform(transform);
		}
		{
			Transform transform = player2Turret.GetWorldTransform();
			transform.position = Lerp(player2TurretLerpPos, transform.position, 0.65f);
			transform.orientation = Slerp(player2TurretLerpOri, transform.orientation, 0.65f);
			player2Turret.SetLocalTransform(transform);
		}
	}

	void TankAIImmobile::FrameUpdate(Room* room, Entity entity, real32 dt)
	{
		Transform transform = entity.GetWorldTransform();
		if (!player1Tank.IsValid())
			return;

		Transform tankTransform = player1Tank.GetWorldTransform();

		lastFireTime += dt;

		Ray viewRay = {};
		viewRay.direction = Normalize(tankTransform.position - transform.position);
		viewRay.origin = transform.position;

		RaycastResult result = room->ShootRayThrough(viewRay);
		if (result.hit && result.entity == player1Tank)
		{
			Vec2f dir = Vec2f(Sin(rotation), Cos(rotation));
			Vec2f look = Normalize(Vec2f(viewRay.direction.x, viewRay.direction.z));

			if (Abs(Dot(dir, look)) < 0.998f)
			{
				int32 sign = Sign(Cross2DScalar(look, dir));
				real32 theta = SCANNING_LOCKED_RATE * dt * sign;

				rotation = NormalizeEulerRotation(rotation + theta);
				transform.LocalRotateY(theta);

				Debug::DrawLine(transform.position, result.entity.GetWorldTransform().position);
			}
			else if (lastFireTime > FIRE_RATE)
			{
				lastFireTime = 0.0f;
				SpawnBullet(room, bulletSpawnPoint.GetWorldTransform());
			}
		}
		else
		{
			real32 delta = SCANNING_TURN_RATE * dt;
			rotation = NormalizeEulerRotation(rotation + delta);
			transform.LocalRotateY(delta);
		}

		entity.SetLocalTransform(transform);

		Sphere collider = tank.GetSphereColliderWorld();
		for (uint32 i = 0; i < room->bullets.count; i++)
		{
			Sphere sphere = room->bullets[i].GetSphereColliderWorld();
			if (CheckIntersectionSphere(sphere, collider))
			{
				PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/explosion_large_01.wav", false);
				room->DestoryEntity(tank);
				room->DestoryEntity(entity);
				break;
			}
		}


	}

}
