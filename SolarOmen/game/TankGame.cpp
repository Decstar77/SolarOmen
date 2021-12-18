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
			brainComponents[index].enabled = false;
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

	Entity Room::SpawnEnemyTank(const Vec3f& pos)
	{
		Entity tankAI = CreateEntity("TankAI");
		tankAI.SetRendering("TankBase", "Tank_DefaultMaterial_BaseColor");
		tankAI.SetLocalTransform(Transform(pos, EulerToQuat(Vec3f(0, 180, 0)), Vec3f(0.65f)));
		tankAI.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));

		Entity turretAI = CreateEntity("Turret");
		turretAI.SetRendering("TankTurret", "Tank_DefaultMaterial_BaseColor");
		turretAI.SetLocalTransform(Transform(Vec3f(pos.x, pos.y + 0.65f, pos.z), Quatf(), Vec3f(0.65f)));

		Entity bulletSpawnPoint = CreateEntity("BulletSpawnPoint");
		bulletSpawnPoint.SetLocalTransform(Transform(Vec3f(0, 0.6f, 1.3f)));
		bulletSpawnPoint.SetCollider(CreateSphere(Vec3f(0, 0, 0), 0.1f));
		bulletSpawnPoint.SetParent(turretAI);

		BrainComponent* brain = turretAI.SetBrain(BrainType::Value::TANK_AI_IMMOBILE);
		brain->tankAIImmobile.tank = tankAI;
		brain->tankAIImmobile.bulletSpawnPoint = bulletSpawnPoint;

		return turretAI;
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
					if (hit)
					{
						Debug::DrawAABB(box);
					}
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
		twoPlayerGame = 0;

		GetPlatofrmState();
		CreateEntityFreeList();

		Entity visualTank = CreateEntity("HostVisualTank");
		visualTank.SetModel("TankBase");
		visualTank.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualTank.SetLocalTransform(Transform(Vec3f(0, 0.1f, 0), Quatf(), Vec3f(0.65f)));

		Entity visualTurret = CreateEntity("HostTurret");
		visualTurret.EnableRendering();
		visualTurret.SetModel("TankTurret");
		visualTurret.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualTurret.SetLocalTransform(Transform(Vec3f(0, 1.1f, 0), EulerToQuat(Vec3f(-8, 0, 0)), Vec3f(0.65f)));

		Entity hostTank = CreateEntity("HostTank");
		hostTank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));
		PlayerBrain* playerBrain = &hostTank.SetBrain(BrainType::Value::PLAYER_BRAIN)->playerBrain;
		playerBrain->visualTank = visualTank;
		playerBrain->visualTurret = visualTurret;

		this->hostTank = hostTank;
		this->hostVisualTank = visualTank;
		this->hostVisualTurret = visualTurret;

		if (twoPlayerGame)
		{
			multiplayerState.processTick = 1;
		}
		else
		{
		}

		{
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

			playerCameraOffset = playerCamera.transform.position - visualTank.GetWorldTransform().position;
		}

		if (twoPlayerGame)
		{
			Entity visualPeerTank = CreateEntity("PeerVisualTank2");
			visualPeerTank.SetModel("TankBase");
			visualPeerTank.SetTexture("Tank_DefaultMaterial_BaseColor");
			visualPeerTank.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(0.65f)));

			Entity visualPeerTurret = CreateEntity("PeerVisualTurret");
			visualPeerTurret.EnableRendering();
			visualPeerTurret.SetModel("TankTurret");
			visualPeerTurret.SetTexture("Tank_DefaultMaterial_BaseColor");
			visualPeerTurret.SetLocalTransform(Transform(Vec3f(0, 1.1f, 0), EulerToQuat(Vec3f(0, 0, -4)), Vec3f(0.65f)));

			Entity peerTank = CreateEntity("PeerTank");
			peerTank.SetCollider(CreateSphere(Vec3f(0, 0, 0), 1.0f));

			this->peerTank = peerTank;
			this->peerVisualTank = visualPeerTank;
			this->peerVisualTurret = visualPeerTurret;

			BrainComponent* brain = peerTank.SetBrain(BrainType::Value::PEER_BRAIN);
			brain->networkBrain.peerTank = peerTank;
			brain->networkBrain.visualPeerTank = visualPeerTank;
			brain->networkBrain.visualPeerTurret = visualPeerTurret;
		}

		if (1)
		{
			//SpawnEnemyTank(Vec3f(7.0f, 0.1f, 7.0f));
			SpawnEnemyTank(Vec3f(-7.0f, 0.1f, -7.0f));
			//SpawnEnemyTank(Vec3f(7.0f, 0.1f, -7.0f));
			//SpawnEnemyTank(Vec3f(-7.0f, 0.1f, 7.0f));
		}

		//Entity prop = CreateEntity("Prop01");
		//prop.EnableRendering();
		//prop.SetModel("Prop_01");
		//prop.SetTexture("Prop_01_DefaultMaterial_BaseColor");
		//prop.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(1)));


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

		Entity visualBullet = room->CreateEntity("VisualBullet");
		visualBullet.SetLocalTransform(transform);
		visualBullet.SetModel("sphere");

		Entity bullet = room->CreateEntity("Bullet");
		bullet.SetCollider(CreateSphere(Vec3f(0), 0.2f));
		bullet.SetLocalTransform(transform);

		room->bullets.Add(visualBullet);

		BulletBrain* brain = &bullet.SetBrain(BrainType::Value::BULLET)->bulletBrain;
		brain->visualBullet = visualBullet;
		brain->bullet = bullet;

		PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/gun_revolver_pistol_shot_04.wav", false);
	}

	static void ApplyGameUpdateInput2Player(Room* room, GameUpdate* gameUpdate)
	{
		Transform t = {};
		if (room->isPlayer1)
		{
			t = room->hostTank.GetLocalTransform();
			t.position = gameUpdate->player1TankPos;
			t.orientation = gameUpdate->player1TankOri;
			room->hostTank.SetLocalTransform(t);

			PeerBrain* brain = &room->peerTank.GetBrain()->networkBrain;
			brain->player2TankLerpPos = gameUpdate->player2TankPos;
			brain->player2TankLerpOri = gameUpdate->player2TankOri;
			brain->player2TurretLerpPos = gameUpdate->player2TurretPos;
			brain->player2TurretLerpOri = gameUpdate->player2TurretOri;

			t.position = gameUpdate->player2TankPos;
			t.orientation = gameUpdate->player2TankOri;
			brain->peerTank.SetLocalTransform(t);
		}
		else
		{
			t = room->hostTank.GetLocalTransform();
			t.position = gameUpdate->player2TankPos;
			t.orientation = gameUpdate->player2TankOri;
			room->hostTank.SetLocalTransform(t);

			PeerBrain* brain = &room->peerTank.GetBrain()->networkBrain;
			brain->player2TankLerpPos = gameUpdate->player1TankPos;
			brain->player2TankLerpOri = gameUpdate->player1TankOri;
			brain->player2TurretLerpPos = gameUpdate->player1TurretPos;
			brain->player2TurretLerpOri = gameUpdate->player1TurretOri;

			t.position = gameUpdate->player1TankPos;
			t.orientation = gameUpdate->player1TankOri;
			brain->peerTank.SetLocalTransform(t);
		}

		if (gameUpdate->player1SpawnBullet)
		{
			Transform transform = {};
			transform.position = RotatePointLHS(gameUpdate->player1TurretOri, Vec3f(0, 0.4f, 1.1f)) + gameUpdate->player1TurretPos;
			transform.orientation = gameUpdate->player1TurretOri;
			SpawnBullet(room, transform);
		}

		if (gameUpdate->player2SpawnBullet)
		{
			Transform transform = {};
			transform.position = gameUpdate->player2TurretPos;
			transform.orientation = gameUpdate->player2TurretOri;
			SpawnBullet(room, transform);
		}
	}

	static void ApplyGameUpdateInputSinglePlayer(Room* room, GameUpdate* gameUpdate)
	{
		Transform t = {};
		t = room->hostTank.GetLocalTransform();
		t.position = gameUpdate->player1TankPos;
		t.orientation = gameUpdate->player1TankOri;
		room->hostTank.SetLocalTransform(t);

		if (gameUpdate->player1SpawnBullet)
		{
			Transform transform = {};
			transform.position = RotatePointLHS(gameUpdate->player1TurretOri, Vec3f(0, 0.4f, 1.1f)) + gameUpdate->player1TurretPos;
			transform.orientation = gameUpdate->player1TurretOri;
			SpawnBullet(room, transform);
		}
	}

	static void TickGame(Room* room, GameUpdate* gameUpdate, real32 dt)
	{
		if (room->twoPlayerGame)
		{
			ApplyGameUpdateInput2Player(room, gameUpdate);
		}
		else
		{
			ApplyGameUpdateInputSinglePlayer(room, gameUpdate);
		}

		for (uint32 i = 0; i < room->brainComponents.GetCapcity(); i++)
		{
			BrainComponent* bc = &room->brainComponents[i];
			if (bc->enabled)
			{
				if (room->entities[i].IsValid())
				{
					Entity entity = room->entities[i];
					switch (bc->type.Get())
					{
					case BrainType::Value::BULLET: bc->bulletBrain.TickUpdate(room, gameUpdate, entity, dt); break;
					case BrainType::Value::TANK_AI_IMMOBILE: bc->tankAIImmobile.TickUpdate(room, gameUpdate, entity, dt); break;
					}
				}
			}
		}
	}

	void Room::Update(real32 dt)
	{
		GameUpdate* gameUpdate = nullptr;
		real32 tickDt = 0.0f;
		if (twoPlayerGame)
		{
			gameUpdate = multiplayerState.GetNextGameplayUpdate(this, dt);
			tickDt = 1.0f / MultiplayerState::TICKS_PER_SECOND;
		}
		else
		{
			tickDt = dt;

			gameUpdate = GameMemory::PushTransientStruct<GameUpdate>();
			Transform transform = hostVisualTank.GetWorldTransform();
			gameUpdate->player1TankPos = transform.position;
			gameUpdate->player1TankOri = transform.orientation;

			transform = hostVisualTurret.GetWorldTransform();
			gameUpdate->player1TurretPos = transform.position;
			gameUpdate->player1TurretOri = transform.orientation;

			gameUpdate->player1SpawnBullet = spawnBullet;
			player1Tank = hostTank;
			spawnBullet = false;
		}

		// @NOTE: No gaurantee that, when playing mutliplayer, we can a game update
		if (gameUpdate)
		{
			TickGame(this, gameUpdate, tickDt);
		}


		for (uint32 i = 0; i < brainComponents.GetCapcity(); i++)
		{
			BrainComponent* bc = &brainComponents[i];
			if (bc->enabled)
			{
				if (entities[i].IsValid())
				{
					switch (bc->type.Get())
					{
					case BrainType::Value::PLAYER_BRAIN: bc->playerBrain.FrameUpdate(this, entities[i], dt);  break;
					case BrainType::Value::BULLET: bc->bulletBrain.FrameUpdate(this, entities[i], dt);  break;
					case BrainType::Value::PEER_BRAIN: bc->networkBrain.FrameUpdate(this, entities[i], dt);  break;
					case BrainType::Value::TANK_AI_IMMOBILE: bc->tankAIImmobile.FrameUpdate(this, entities[i], dt);  break;
					}
				}
			}
		}

		//DEBUGDrawAllColliders();
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

	void PlayerBrain::TickUpdate(Room* room, GameUpdate* update, Entity entity, real32 dt)
	{
		Debug::DrawSphere(entity.GetSphereColliderWorld());
	}

	void PlayerBrain::FrameUpdate(Room* room, Entity entity, real32 dt)
	{
		GetInput();

		Transform tankTransform = visualTank.GetWorldTransform();

		int32 turnDir = input->d - input->a;
		int32 moveDir = input->w - input->s;

		real32 rotationDelta = turnDir * TANK_ROTATION_SPEED * dt;
		visualTankRotation += rotationDelta;
		tankTransform.LocalRotateY(rotationDelta);

		real32 moveDelta = moveDir * TANK_MOVE_SPEED * dt;
		Vec2f forwardDir = Vec2f(Sin(visualTankRotation), Cos(visualTankRotation));
		tankTransform.position += Vec3f(moveDelta * forwardDir.x, 0.0f, moveDelta * forwardDir.y);

		// @NOTE: Collision detection 
		Sphere tankCollider = CreateSphere(tankTransform.position, 0.8f);
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
			}
		}

		visualTank.SetLocalTransform(tankTransform);
		room->playerCamera.transform.position = Lerp(room->playerCamera.transform.position,
			tankTransform.position + room->playerCameraOffset, 0.1f);

		UpdateTurret(room, dt);
		UpdateFiring(room, dt);
	}

	void PlayerBrain::UpdateTurret(Room* room, real32 dt)
	{
		Transform turretTransform = visualTurret.GetLocalTransform();

		Ray ray = room->playerCamera.ShootRayFromScreen();
		RaycastInfo rayInfo = {};
		if (RaycastPlane(ray, CreatePlane(Vec3f(0), Vec3f(0, 1, 0)), &rayInfo))
		{
			Vec2f point = Vec2f(rayInfo.closePoint.x, rayInfo.closePoint.z);
			Vec3f worldPos = visualTurret.GetWorldTransform().position;

			Vec2f look = point - Vec2f(worldPos.x, worldPos.z);
			Vec2f dir = Vec2f(Sin(visualTurretRotation), Cos(visualTurretRotation));

			look = Normalize(look);
			dir = Normalize(dir);

			//@NOTE: 2D Cross product
			int32 sign = Sign(Cross2DScalar(look, dir));

			//Debug::DrawPoint(rayInfo.closePoint + Vec3f(0, worldPos.y, 0));
			//Debug::DrawLine(worldPos, worldPos + 100.0f * Vec3f(look.x, 0.0f, look.y));
			//Debug::DrawLine(worldPos, worldPos + 100.0f * Vec3f(dir.x, 0.0f, dir.y));

			real32 theta = Acos(Clamp(Dot(look, dir), 0.0f, 1.0f)) * sign;

			visualTurretRotation = NormalizeEulerRotation(visualTurretRotation + theta);

			turretTransform.LocalRotateY(theta);
		}

		turretTransform.position = visualTank.GetWorldTransform().position + Vec3f(0.0f, 0.65f, 0.0f);

		visualTurret.SetLocalTransform(turretTransform);
	}

	void PlayerBrain::UpdateBase(Room* room, real32 dt)
	{

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
				room->spawnBullet = true;
			}
			else
			{
				PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/gun_rifle_dry_fire_01.wav", false);
			}
		}
	}

	void BulletBrain::TickUpdate(Room* room, GameUpdate* update, Entity entity, real32 dt)
	{
		Transform transform = bullet.GetLocalTransform();
		Basisf basis = transform.GetBasis();

		moveDir = Normalize(Vec2(basis.forward.x, basis.forward.z));
		moveDelta = BULLET_MOVE_SPEED * moveDir * dt;

		transform.position += Vec3f(moveDelta.x, 0.0f, moveDelta.y);

		// @NOTE: Collision detection 
		bool hitCollider = false;
		Sphere bulletCollider = bullet.GetSphereColliderLocal();
		bulletCollider = TranslateSphere(bulletCollider, transform.position);
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

		bullet.SetLocalTransform(transform);


		if (hitCollider)
			collisionCount++;

		if (collisionCount == 2)
		{
			room->DestoryEntity(bullet);
			room->DestoryEntity(visualBullet);
		}
	}

	void BulletBrain::FrameUpdate(Room* room, Entity entity, real32 dt)
	{
		Transform visualTransform = visualBullet.GetLocalTransform();
		visualTransform.position = Lerp(bullet.GetWorldTransform().position, visualTransform.position, NETWORK_INTERPOLATE_AMOUNT);
		visualBullet.SetLocalTransform(visualTransform);
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
			Transform transform = visualPeerTank.GetWorldTransform();
			transform.position = Lerp(player2TankLerpPos, transform.position, NETWORK_INTERPOLATE_AMOUNT);
			transform.orientation = Slerp(player2TankLerpOri, transform.orientation, NETWORK_INTERPOLATE_AMOUNT);
			visualPeerTank.SetLocalTransform(transform);
		}
		{
			Transform transform = visualPeerTurret.GetWorldTransform();
			transform.position = Lerp(player2TurretLerpPos, transform.position, NETWORK_INTERPOLATE_AMOUNT);
			transform.orientation = Slerp(player2TurretLerpOri, transform.orientation, NETWORK_INTERPOLATE_AMOUNT);
			visualPeerTurret.SetLocalTransform(transform);
		}
	}

	void TankAIImmobile::TickUpdate(Room* room, GameUpdate* update, Entity entity, real32 dt)
	{
		Transform transform = entity.GetLocalTransform();

		real32 dist = 0.0f;
		Entity shootTank = {};
		if (room->player1Tank.IsValid())
		{
			dist = DistanceSqrd(room->player1Tank.GetWorldTransform().position, transform.position);
			shootTank = room->player1Tank;
		}
		if (room->player2Tank.IsValid())
		{
			real32 d2 = DistanceSqrd(room->player2Tank.GetWorldTransform().position, transform.position);
			if (d2 < dist)
			{
				shootTank = room->player2Tank;
			}
		}

		if (!shootTank.IsValid())
			return;

		Transform tankTransform = shootTank.GetWorldTransform();

		lastFireTime += dt;

		Ray viewRay = {};
		viewRay.direction = Normalize(tankTransform.position - transform.position);
		viewRay.origin = transform.position;

		RaycastResult result = room->ShootRayThrough(viewRay);
		if (result.hit && (result.entity.GetBrain()->type == BrainType::Value::PLAYER_BRAIN
			|| result.entity.GetBrain()->type == BrainType::Value::PEER_BRAIN))
		{
			Vec2f dir = Vec2f(Sin(rotation), Cos(rotation));
			Vec2f look = Normalize(Vec2f(viewRay.direction.x, viewRay.direction.z));

			if (Abs(Dot(dir, look)) < 0.999f)
			{
				int32 sign = Sign(Cross2DScalar(look, dir));
				real32 theta = SCANNING_LOCKED_RATE * dt * sign;

				rotation = NormalizeEulerRotation(rotation + theta);
				transform.LocalRotateY(theta);

				//Debug::DrawLine(transform.position, result.entity.GetWorldTransform().position);
			}
			else if (lastFireTime > FIRE_RATE)
			{
				lastFireTime = 0.0f;
				transform.LookAtLH(transform.position + Vec3f(look.x, 0.0f, look.y));
				entity.SetLocalTransform(transform);

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


		//Sphere collider = tank.GetSphereColliderWorld();
		//for (uint32 i = 0; i < room->brainComponents.GetCapcity(); i++)
		//{
		//	BrainComponent* bc = &room->brainComponents[i];
		//	if (bc->enabled)
		//	{
		//		if (room->entities[i].IsValid())
		//		{
		//			switch (bc->type.Get())
		//			{
		//			case BrainType::Value::BULLET:
		//			{
		//				if (CheckIntersectionSphere(bc->bulletBrain.bullet.GetSphereColliderWorld(), collider))
		//				{
		//					PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/explosion_large_01.wav", false);
		//					room->DestoryEntity(tank);
		//					room->DestoryEntity(entity);
		//					room->DestoryEntity(bc->bulletBrain.bullet);
		//					room->DestoryEntity(bc->bulletBrain.visualBullet);
		//					break;
		//				}
		//			}break;
		//			}
		//		}
		//	}
		//}


		//for (uint32 i = 0; i < room->bullets.count; i++)
		//{
		//	Sphere sphere = room->bullets[i].GetSphereColliderWorld();
		//	if (CheckIntersectionSphere(sphere, collider))
		//	{
		//		PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/explosion_large_01.wav", false);
		//		room->DestoryEntity(tank);
		//		room->DestoryEntity(entity);
		//		break;
		//	}
		//}
	}

	void TankAIImmobile::FrameUpdate(Room* room, Entity entity, real32 dt)
	{

	}



}
