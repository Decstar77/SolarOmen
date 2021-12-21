#include "TankGame.h"
#include "core/SolarCore.h"
#include "core/SolarGame.h"


namespace cm
{
	static bool DEBUG_CALL_MUST_HAPPEN_ON_TICK = false;
#ifdef DEBUG
	// @NOTE: Means that this function is being called through an on, "on frame update". Use a command instead !!
#define CHECK_CALLER_IS_ON_TICK() Assert(DEBUG_CALL_MUST_HAPPEN_ON_TICK, "Called not on tick");
#define ENABLE_CALLS_ON_TICK() DEBUG_CALL_MUST_HAPPEN_ON_TICK = true;
#define DISABLE_CALLS_ON_TICK() DEBUG_CALL_MUST_HAPPEN_ON_TICK = false;
#else
#define	CHECK_CALLER_IS_ON_TICK()
#endif

	Entity Room::CreateEntity()
	{
		CHECK_CALLER_IS_ON_TICK();

		EntityId id = GetNextFreeEntityId();
		Assert(id.index > 0 && id.index < (int32)entities.GetCapcity(), "Entity id index was invalid");

		//LOG("Creating entity: " << id.index);

		Entity* entity = &entities[id.index];
		Assert(entity->id.index == INVALID_ENTITY_INDEX, "Using a valid entity");
		entity->id = id;
		entity->room = this;

		transformComponents[id.index].transform = Transform();

		BrainComponent* bc = &brainComponents[id.index];
		ZeroStruct(bc);

		NetworkComponent* nc = &networkComponents[id.index];
		ZeroStruct(nc);

		return *entity;
	}

	void Room::DestoryEntity(Entity entity)
	{
		CHECK_CALLER_IS_ON_TICK();

		if (entity.IsValid())
		{
			//LOG("Destorying entity: " << entity.id.index);

			PushFreeEntityId(entity.id);

			int32 index = entity.id.index;
			ZeroStruct(&entities[index]);
			ZeroStruct(&nameComponents[index]);
			ZeroStruct(&renderComponents[index]);
			ZeroStruct(&colliderComponents[index]);
			ZeroStruct(&tagComponents[index]);

			// @NOTE: We don't zero the brain beacause if the brain is calling this function it will
			//		: mess the current state of the brain, and thus any futher code is be broken entirely
			//		: instead what happens is the brain is zero'd upon being set.
			brainComponents[index].enabled = false;
			//ZeroStruct(&brainComponents[index]);
		}
	}

	Entity Room::CreateEntity(const CString& name)
	{
		Entity entity = CreateEntity();
		entity.SetName(name);

		LOG("Creating entity: " << entity.id.index << " " << name.GetCStr());

		return entity;
	}

	void Room::GameCommandSpawnBullet(const Vec3f& pos, const Quatf& ori)
	{
		GameCommand command = {};
		command.type = GameCommandType::SPAWN_BULLET;
		command.spawnBullet.pos = pos;
		command.spawnBullet.ori = ori;

		commands.Add(command);
	}

	void Room::GameCommandDestroyEntity(Entity entity)
	{
		Assert(entity.IsValid(), "");
		networkComponents[entity.id.index].markedDestroyed = true;

		GameCommand cmd = {};
		cmd.type = GameCommandType::DESTROY_ENTITY;
		cmd.destroyEntity.id = entity.GetId();

		commands.Add(cmd);
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

	Entity Room::SpawnBullet(Transform transform)
	{
		transform.scale = Vec3f(0.2f);
		Basisf basis = transform.GetBasis();
		basis.forward.y = 0.0f;
		basis.forward = Normalize(basis.forward);
		basis.upward = Vec3f(0, 1, 0);
		basis.right = Cross(basis.upward, basis.forward);
		transform.orientation = Mat3ToQuat(basis.mat);

		Entity bullet = CreateEntity("Bullet");
		bullet.SetCollider(CreateSphere(Vec3f(0), 0.2f));
		bullet.SetLocalTransform(transform);
		bullet.SetModel("sphere");
		BulletBrain* brain = &bullet.SetBrain(BrainType::Value::BULLET)->bulletBrain;

		networkComponents[bullet.id.index].lerpPosition = transform.position;
		networkComponents[bullet.id.index].lerpOrientation = transform.orientation;

		//for (uint32 i = 0; i < bullets.GetCapcity(); i++)
		//{
		//	if (!bullets[i].IsValid())
		//	{
		//		brain->index = i;
		//		bullets[i] = bullet;
		//	}
		//}

		PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/gun_revolver_pistol_shot_04.wav", false);

		return bullet;
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

	void Room::CreateHostTank()
	{
		GetPlatofrmState();
		Entity visualTank = CreateEntity("HostVisualTank");
		visualTank.SetModel("TankBase");
		visualTank.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualTank.SetLocalTransform(Transform(Vec3f(0, 0.1f, 0), Quatf(), Vec3f(0.65f)));
		visualTank.SetNetworkOwner(multiplayerState.playerNumber);
		visualTank.SetTag(Tag::Value::TEAM1_TANK);
		visualTank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));

		Entity visualTurret = CreateEntity("HostVisualTurret");
		visualTurret.EnableRendering();
		visualTurret.SetModel("TankTurret");
		visualTurret.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualTurret.SetLocalTransform(Transform(Vec3f(0, 1.1f, 0), EulerToQuat(Vec3f(-8, 0, 0)), Vec3f(0.65f)));
		visualTurret.SetNetworkOwner(multiplayerState.playerNumber);

		Entity hostTank = CreateEntity("HostTank");
		hostTank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));
		hostTank.SetNetworkOwner(multiplayerState.playerNumber);
		PlayerBrain* playerBrain = &hostTank.SetBrain(BrainType::Value::PLAYER_BRAIN)->playerBrain;
		playerBrain->visualTank = visualTank;
		playerBrain->visualTurret = visualTurret;

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

	void Room::CreatePeerTank()
	{
		Entity visualPeerTank = CreateEntity("PeerVisualTank");
		visualPeerTank.SetModel("TankBase");
		visualPeerTank.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualPeerTank.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(0.65f)));
		visualPeerTank.SetNetworkOwner(GetOppositePlayer(multiplayerState.playerNumber));
		visualPeerTank.SetTag(Tag::Value::TEAM1_TANK);
		visualPeerTank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));

		Entity visualPeerTurret = CreateEntity("PeerVisualTurret");
		visualPeerTurret.EnableRendering();
		visualPeerTurret.SetModel("TankTurret");
		visualPeerTurret.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualPeerTurret.SetLocalTransform(Transform(Vec3f(0, 1.1f, 0), EulerToQuat(Vec3f(0, 0, -4)), Vec3f(0.65f)));
		visualPeerTurret.SetNetworkOwner(GetOppositePlayer(multiplayerState.playerNumber));

		Entity peerTank = CreateEntity("PeerTank");
	}

	void Room::Initialize()
	{
		if (initialized)
			return;

		ENABLE_CALLS_ON_TICK();

		CreateEntityFreeList();
		if (twoPlayerGame)
		{
			multiplayerState.tickCounter = 1;
			// @NOTE: Ensure that entity id's line up
			if (multiplayerState.playerNumber == PlayerNumber::ONE)
			{
				CreateHostTank();
				CreatePeerTank();
			}
			else if (multiplayerState.playerNumber == PlayerNumber::TWO)
			{
				CreatePeerTank();
				CreateHostTank();
			}
		}
		else
		{
			CreateHostTank();
		}

		if (1)
		{
			SpawnEnemyTank(Vec3f(7.0f, 0.1f, 7.0f)).SetNetworkOwner(PlayerNumber::ONE);
			//SpawnEnemyTank(Vec3f(-7.0f, 0.1f, -7.0f)).SetNetworkOwner(PlayerNumber::ONE);
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

		initialized = true;

		DISABLE_CALLS_ON_TICK();
	}

	void Room::Initialize(bool32 twoPlayer)
	{
		twoPlayerGame = 0;
		if (!twoPlayerGame)
		{
			multiplayerState.playerNumber = PlayerNumber::ONE;
			Initialize();
		}
	}

	void Room::PerformGameCommands(FixedArray<GameCommand, 256>* commands, PlayerNumber playerNumber)
	{
		for (uint32 i = 0; i < commands->count; i++)
		{
			GameCommand* command = commands->Get(i);
			switch (command->type)
			{
			case GameCommandType::SPAWN_BULLET:
			{
				Entity ent = SpawnBullet(Transform(command->spawnBullet.pos, command->spawnBullet.ori));
				ent.SetNetworkOwner(playerNumber);
			}break;
			case GameCommandType::DESTROY_ENTITY:
			{
				Entity* entity = command->destroyEntity.id.Get();
				if (entity)
				{
					DestoryEntity(*entity);
				}
			}break;
			}
		}
	}

	void Room::Update(real32 dt)
	{
		if (!initialized && multiplayerState.IsValid())
			Initialize();

		if (twoPlayerGame)
		{
			GameCommands* gameCommands = multiplayerState.GetNextGameCommands(this, dt);
			if (gameCommands)
			{
				ENABLE_CALLS_ON_TICK();
				PerformGameCommands(&gameCommands->player1Commands, PlayerNumber::ONE);
				PerformGameCommands(&gameCommands->player2Commands, PlayerNumber::TWO);
				DISABLE_CALLS_ON_TICK();
			}
		}
		else
		{
			ENABLE_CALLS_ON_TICK();
			PerformGameCommands(&commands, PlayerNumber::ONE);
			DISABLE_CALLS_ON_TICK();
			commands.Clear();
		}

		for (uint32 i = 0; i < transformComponents.GetCapcity(); i++)
		{
			TransformComponent* comp = &transformComponents[i];
			NetworkComponent* net = &networkComponents[i];
			if (net->playerOwner == GetOppositePlayer(multiplayerState.playerNumber)
				&& entities[i].IsValid())
			{
				Transform* transform = &comp->transform;
				transform->position = Lerp(net->lerpPosition, transform->position, NETWORK_INTERPOLATE_AMOUNT);
				transform->orientation = Slerp(net->lerpOrientation, transform->orientation, NETWORK_INTERPOLATE_AMOUNT);
			}
		}

		for (uint32 i = 0; i < brainComponents.GetCapcity(); i++)
		{
			BrainComponent* bc = &brainComponents[i];
			NetworkComponent* net = &networkComponents[i];
			if (bc->enabled && net->playerOwner == multiplayerState.playerNumber && !net->markedDestroyed)
			{
				if (entities[i].IsValid())
				{
					switch (bc->type.Get())
					{
					case BrainType::Value::PLAYER_BRAIN: bc->playerBrain.FrameUpdate(this, entities[i], dt);  break;
					case BrainType::Value::BULLET: bc->bulletBrain.FrameUpdate(this, entities[i], dt);  break;
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
		gs->currentRoom.Initialize(false);

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

	void PlayerBrain::FrameUpdate(Room* room, Entity entity, real32 dt)
	{
		UpdateBase(room, dt);
		UpdateTurret(room, dt);
		UpdateFiring(room, dt);
	}

	void PlayerBrain::UpdateBase(Room* room, real32 dt)
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

		CompressedQuatf comp = CompressQuatf(tankTransform.orientation);
		tankTransform.orientation = DecompressQuatf(comp);

		visualTank.SetLocalTransform(tankTransform);
		room->playerCamera.transform.position = Lerp(room->playerCamera.transform.position,
			tankTransform.position + room->playerCameraOffset, 0.1f);
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

				Transform t = visualTurret.GetWorldTransform();
				room->GameCommandSpawnBullet(t.position, t.orientation);
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

		moveDir = Normalize(Vec2(basis.forward.x, basis.forward.z));
		moveDelta = BULLET_MOVE_SPEED * moveDir * dt;

		transform.position += Vec3f(moveDelta.x, 0.0f, moveDelta.y);

		// @NOTE: Collision detection 
		bool hitCollider = false;
		Sphere bulletCollider = entity.GetSphereColliderLocal();
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

		entity.SetLocalTransform(transform);


		if (hitCollider)
			collisionCount++;

		if (collisionCount == 2)
		{
			room->GameCommandDestroyEntity(entity);
		}
	}

	void TankAIImmobile::FrameUpdate(Room* room, Entity entity, real32 dt)
	{
		Transform transform = entity.GetLocalTransform();

		Entity shootTank = {};
		room->BeginEntityLoop();
		real32 dist = 100000;
		while (Entity ent = room->GetNextEntity())
		{
			if (ent.GetTag() == Tag::Value::TEAM1_TANK)
			{
				real32 d = DistanceSqrd(ent.GetWorldTransform().position, transform.position);
				if (d < dist)
				{
					dist = d;
					shootTank = ent;
				}
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
		if (result.hit && result.entity == shootTank)
		{
			Vec2f dir = Vec2f(Sin(rotation), Cos(rotation));
			Vec2f look = Normalize(Vec2f(viewRay.direction.x, viewRay.direction.z));

			if (Abs(Dot(dir, look)) < 0.99f)
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
				transform.LookAtLH(transform.position + Vec3f(look.x, 0.0f, look.y));
				entity.SetLocalTransform(transform);

				Transform t = bulletSpawnPoint.GetWorldTransform();
				room->GameCommandSpawnBullet(t.position, t.orientation);

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



}
