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
#define ENABLE_CALLS_ON_TICK() 
#define DISABLE_CALLS_ON_TICK()
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

	void Room::RemoveEntityChildParentRelationship(Entity* entity)
	{
		Assert(entity->IsValid(), "RemoveEntityChildParentRelationship");

		for (Entity* child = entity->GetFirstChild(); child != nullptr;)
		{
			// @NOTE: Get the sibling before we delete/clear the current entity 
			Entity* sibling = child->GetSiblingAhead();
			DestoryEntity(child);
			child = sibling;
		}

		if (Entity* parent = entity->GetParent())
		{
			Assert(*parent->child.Get() == *entity, "RemoveEntityChildParentRelationship");
			parent->child = entity->siblingAhead;
		}

		if (Entity* sibling = entity->GetSiblingAhead())
		{
			Assert(*sibling->GetSiblingBehind() == *entity, "RemoveEntityChildParentRelationship");
			sibling->siblingBehind = entity->siblingBehind;
		}

		if (Entity* sibling = entity->GetSiblingBehind())
		{
			Assert(*sibling->GetSiblingAhead() == *entity, "RemoveEntityChildParentRelationship");
			sibling->siblingAhead = entity->siblingAhead;
		}

		// @NOTE: Update this pointer to what ever changed
		*entity = *entity->GetId().Get();
	}

	void Room::DestoryEntity(Entity* entity)
	{
		Assert(entity, "Destroy null entity ?");

		CHECK_CALLER_IS_ON_TICK();

		if (entity && entity->IsValid())
		{
			//LOG("Destorying entity: " << entity.id.index);
			RemoveEntityChildParentRelationship(entity);

			PushFreeEntityId(entity->id);

			int32 index = entity->id.index;
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

			*entity = {};
		}
	}

	Entity Room::CreateEntity(const CString& name)
	{
		Entity entity = CreateEntity();
		entity.SetName(name);

		//LOG("Creating entity: " << entity.id.index << " " << name.GetCStr());

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

			if (entityLoopIndex == entities.count)
			{
				return entity;
			}
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
		bullet.SetTag(Tag::Value::BULLET);
		BulletBrain* brain = &bullet.SetBrain(BrainType::Value::BULLET)->bulletBrain;

		networkComponents[bullet.id.index].lerpPosition = transform.position;
		networkComponents[bullet.id.index].lerpOrientation = transform.orientation;

		PlaySound("gun_revolver_pistol_shot_04", false);

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
		bulletSpawnPoint.SetParent(&turretAI);

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

	void Room::CreateHostTank(const Vec3f& position)
	{
		GetPlatofrmState();
		Entity visualTank = CreateEntity("HostVisualTank");
		visualTank.SetModel("TankBase");
		visualTank.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualTank.SetLocalTransform(Transform(position, Quatf(), Vec3f(0.65f)));
		visualTank.SetNetworkOwner(multiplayerState->playerNumber);
		visualTank.SetTag(Tag::Value::TEAM1_TANK);
		visualTank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));

		Entity visualTurret = CreateEntity("HostVisualTurret");
		visualTurret.EnableRendering();
		visualTurret.SetModel("TankTurret");
		visualTurret.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualTurret.SetLocalTransform(Transform(position + Vec3f(0, 1.0f, 0), EulerToQuat(Vec3f(-8, 0, 0)), Vec3f(0.65f)));
		visualTurret.SetNetworkOwner(multiplayerState->playerNumber);

		Entity hostTank = CreateEntity("HostTank");
		hostTank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));
		hostTank.SetNetworkOwner(multiplayerState->playerNumber);
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

	void Room::CreatePeerTank(const Vec3f& position)
	{
		Entity visualPeerTank = CreateEntity("PeerVisualTank");
		visualPeerTank.SetModel("TankBase");
		visualPeerTank.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualPeerTank.SetLocalTransform(Transform(position, Quatf(), Vec3f(0.65f)));
		visualPeerTank.SetNetworkOwner(GetOppositePlayer(multiplayerState->playerNumber));
		visualPeerTank.SetTag(Tag::Value::TEAM1_TANK);
		visualPeerTank.SetCollider(CreateSphere(Vec3f(0, 0.5f, 0), 0.8f));

		Entity visualPeerTurret = CreateEntity("PeerVisualTurret");
		visualPeerTurret.EnableRendering();
		visualPeerTurret.SetModel("TankTurret");
		visualPeerTurret.SetTexture("Tank_DefaultMaterial_BaseColor");
		visualPeerTurret.SetLocalTransform(Transform(position + Vec3f(0, 1.0f, 0), EulerToQuat(Vec3f(0, 0, -4)), Vec3f(0.65f)));
		visualPeerTurret.SetNetworkOwner(GetOppositePlayer(multiplayerState->playerNumber));

		Entity peerTank = CreateEntity("PeerTank");
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
					DestoryEntity(entity);
				}
			}break;
			}
		}
	}

	void Room::CreateEntitiesFromGripMap()
	{
		ENABLE_CALLS_ON_TICK();

		DestoryEntity(&gridEntity);
		gridEntity = CreateEntity("Grid");
		for (uint32 i = 0; i < grid.cells.GetCapcity(); i++)
		{
			GridCell* cell = &grid.cells[i];

			switch (cell->type.Get())
			{
			case GridCellType::Value::WALL:
			{
				Entity entity = CreateEntity("Wall");
				entity.SetLocalTransform(Transform(Vec3f(cell->position.x, 0.0f, cell->position.y)));
				entity.SetModel("20x20_Full");
				entity.SetTexture("Set0_Texture");
				entity.EnableCollider();
				entity.SetCollider(CreateAABBFromCenterRadius(Vec3f(0, 1, 0), Vec3f(1)));
				entity.SetParent(&gridEntity);
				cell->entity = entity;
			}break;
			}

			//	{
			//		Entity entity = CreateEntity("EmptyCell");
			//		entity.SetLocalTransform(Transform(Vec3f(cell->position.x, 0.0f, cell->position.y)));
			//		entity.SetModel("20x20_Floor");
			//		entity.SetTexture("Set0_Texture");
			//		entity.SetParent(&gridEntity);
			//		cell->entity = entity;
			//	}
			//	else if (occupied == 1)
			//	{
			//		Entity entity = CreateEntity("Cell");
			//		entity.SetLocalTransform(Transform(Vec3f(cell->position.x, 0.0f, cell->position.y)));
			//		entity.SetModel("20x20_Full");
			//		entity.SetTexture("Set0_Texture");
			//		entity.EnableCollider();
			//		entity.SetCollider(CreateAABBFromCenterRadius(Vec3f(0, 1, 0), Vec3f(1)));
			//		entity.SetParent(&gridEntity);
			//		cell->entity = entity;
			//	}
		}

		//int32 map[] =
		//{
		//	1,1,1,1,1,1,1,1,1,1,
		//	1,0,0,0,0,0,0,0,0,1,
		//	1,0,0,0,1,1,0,0,0,1,
		//	1,0,0,0,1,1,0,0,0,1,
		//	1,1,0,0,0,0,0,0,1,1,
		//	1,1,0,0,0,0,0,0,1,1,
		//	1,0,0,0,1,1,0,0,0,1,
		//	1,0,0,0,1,1,0,0,0,1,
		//	1,0,0,0,0,0,0,0,0,1,
		//	1,1,1,1,1,1,1,1,1,1,
		//};

		//Entity gridEntity = CreateEntity("Grid");
		//for (int32 index = 0; index < ArrayCount(map); index++)
		//{
		//	GridCell* cell = &grid.cells[index];
		//	int32 occupied = map[index];
		//	cell->occupied = occupied;
		//	if (occupied == 0)
		//	{
		//		Entity entity = CreateEntity("EmptyCell");
		//		entity.SetLocalTransform(Transform(Vec3f(cell->position.x, 0.0f, cell->position.y)));
		//		entity.SetModel("20x20_Floor");
		//		entity.SetTexture("Set0_Texture");
		//		entity.SetParent(&gridEntity);
		//		cell->entity = entity;
		//	}
		//	else if (occupied == 1)
		//	{
		//		Entity entity = CreateEntity("Cell");
		//		entity.SetLocalTransform(Transform(Vec3f(cell->position.x, 0.0f, cell->position.y)));
		//		entity.SetModel("20x20_Full");
		//		entity.SetTexture("Set0_Texture");
		//		entity.EnableCollider();
		//		entity.SetCollider(CreateAABBFromCenterRadius(Vec3f(0, 1, 0), Vec3f(1)));
		//		entity.SetParent(&gridEntity);
		//		cell->entity = entity;
		//	}
		//}

		DISABLE_CALLS_ON_TICK();
	}

	/// <summary>
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// </summary>

	struct AABB2D
	{
		Vec2f min;
		Vec2f max;

		inline static AABB2D CreateFromCenterHalfDims(const Vec2f& center, const Vec2f& dims)
		{
			AABB2D result = {};
			result.min = Vec2f(center.x - dims.x, center.y - dims.y);
			result.max = Vec2f(center.x + dims.x, center.y + dims.y);
			return result;
		}

		inline bool IsInside(const Vec2f& pos)
		{
			return (pos.x > min.x) &&
				(pos.y > min.y) &&
				(pos.x < max.x) &&
				(pos.y < max.y);
		}
	};

	void Room::InitializeMenuRoom()
	{
		uiState.selectionPos = 0;
	}

	void Room::UpdateMenuRoom(real32 dt)
	{
		uiState.Start();

		uint32 oldSelection = uiState.selectionPos;

		uiState.totalTime += dt;
		uiState.selectionScale = 0.1f * Sin(uiState.totalTime) + 0.9f;

		GetAssetState();
		auto textures = as->textures.GetValueSet();
		uiState.Rect(0.5f, 0.5f, 0.25f, 0.25f, GetAssetFromName(textures, "MenuBackground").id);
		//uiState.Rect(0.5f, 0.5f, 0.25f, 0.25f, GetAssetFromName(textures, "HUD").id, Vec4f(0.6, 0.6, 0.6, 0.5f));

		uiState.Text("Single Player", 0.5f, 0.4f, uiState.selectionPos == 0 ? uiState.selectionScale : 0.8f);
		uiState.Text("Multiplayer", 0.5f, 0.5f, uiState.selectionPos == 1 ? uiState.selectionScale : 0.8f);
		uiState.Text("Options", 0.5f, 0.6f, uiState.selectionPos == 2 ? uiState.selectionScale : 0.8f);
		uiState.Text("Exit", 0.5f, 0.7f, uiState.selectionPos == 3 ? uiState.selectionScale : 0.8f);

		GetInput();
		uiState.selectionPos += (IsKeyJustDown(input, s) - IsKeyJustDown(input, w));
		uiState.selectionPos %= 4;

		if (oldSelection != uiState.selectionPos)
		{
			PlaySound("ui_menu_slide", false);
			uiState.totalTime = 0.0f;
		}

		if (IsKeyJustDown(input, e))
		{
			GetGameState();
			if (uiState.selectionPos == 0)
			{
				gs->nextRoom = RoomType::LEVEL_SELECTION;
			}
			else if (uiState.selectionPos == 1)
			{
				gs->nextRoom = RoomType::MULTIPLAYER;
			}
			else if (uiState.selectionPos == 2)
			{
				gs->nextRoom = RoomType::OPTIONS;
			}
			else if (uiState.selectionPos == 3)
			{
				Platform::PostQuitMessage();
			}
			PlaySound("ui_menu_select", false);
		}

		//GetPlatofrmState();
		//real32 w = as->font.GetWidthOfText("Exit", uiState.selectionPos == 0 ? uiState.selectionScale : 0.8f);
		//real32 h = as->font.GetHeightOfText("Exit", uiState.selectionPos == 0 ? uiState.selectionScale : 0.8f);
		//AABB2D box = AABB2D::CreateFromCenterHalfDims(Vec2f(0.5f * ps->clientWidth, 0.7f * ps->clientHeight), Vec2f(w / 2.0f, h));
		//if (box.IsInside(input->mousePositionPixelCoords)) { LOG("EX"); }

		uiState.End();
	}

	void Room::ShutdownMenuRoom()
	{
	}

	/// <summary>
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// </summary>

	void Room::InitializeLevelSelectionRoom()
	{

	}

	void Room::UpdateLevelSelectionRoom(real32 dt)
	{
		uiState.Start();
		uiState.Text("Choose a level", 0.5f, 0.1f, 0.9f);

		if (uiState.Button("Back", 0.5f, 0.17f, 0.05f, 0.04f, 0.5f))
		{
			GetGameState();
			gs->nextRoom = RoomType::MAIN_MENU;
			PlaySound("ui_menu_select", false);
		}

		for (real32 x = 0.1f; x <= 1.0f; x += 0.2f)
		{
			for (real32 y = 0.3f; y <= 1.0f; y += 0.2f)
			{
				CString name = CString("Level").Add((int32)(10.0f * x)).Add((int32)(10.0f * y));
				if (uiState.Button(name, x, y, 0.07f, 0.07f, 0.5f))
				{
					GetGameState();
					gs->nextRoom = RoomType::LEVEL_1;
					PlaySound("ui_menu_select", false);
				}
			}
		}

		uiState.End();
	}

	void Room::ShutdownLevelSelectionRoom()
	{

	}

	/// <summary>
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// </summary>

	void Room::InitializeMultiplayerRoom()
	{
		uiState.selectionPos = 0;
	}

	void Room::UpdateMultiplayerRoom(real32 dt)
	{
		uiState.Start();
		uiState.totalTime += dt;

		if (!multiplayerState->startedNetworkStuff)
		{
			uint32 oldSelection = uiState.selectionPos;
			uiState.selectionScale = 0.1f * Sin(uiState.totalTime) + 0.9f;

			uiState.Text("Host", 0.5f, 0.4f, uiState.selectionPos == 0 ? uiState.selectionScale : 0.8f);
			uiState.Text("Connect", 0.5f, 0.5f, uiState.selectionPos == 1 ? uiState.selectionScale : 0.8f);
			uiState.Text("Back", 0.5f, 0.6f, uiState.selectionPos == 2 ? uiState.selectionScale : 0.8f);

			GetInput();

			uiState.selectionPos += (IsKeyJustDown(input, s) - IsKeyJustDown(input, w));
			uiState.selectionPos %= 3;

			if (oldSelection != uiState.selectionPos)
			{
				PlaySound("ui_menu_slide", false);
				uiState.totalTime = 0.0f;
			}

			if (IsKeyJustDown(input, e))
			{
				GetGameState();
				if (uiState.selectionPos == 0)
				{
					multiplayerState->StartHost();
				}
				else if (uiState.selectionPos == 1)
				{
					multiplayerState->StartPeer();
				}
				else if (uiState.selectionPos == 2)
				{
					gs->nextRoom = RoomType::MAIN_MENU;
				}

				PlaySound("ui_menu_select", false);
			}
		}
		else
		{
			multiplayerState->GetNextGameCommands(this, dt);
			if (multiplayerState->connectionValid)
			{
				uiState.Text("Connection established !!", 0.5f, 0.5f, 0.8f);
				GetGameState();
				gs->nextRoom = RoomType::LEVEL_1;
			}
			else if (multiplayerState->playerNumber == PlayerNumber::ONE)
			{
				uiState.Text("Tell your deer freind to connect", 0.5f, 0.3f, 0.8f);
				uiState.Text(CString("Your IP: ").Add(multiplayerState->myAddress.stringIP), 0.5f, 0.4f, 0.8f);
			}
			else if (multiplayerState->playerNumber == PlayerNumber::TWO)
			{
				uiState.Text("Attempting to connect to: 192.168.0.7", 0.5f, 0.3f, 0.8f);
				uiState.Text("...", 0.5f, 0.4f, 0.8f);
			}
		}

		uiState.End();
	}

	void Room::ShutdownMultiplayeRoom()
	{

	}

	/// <summary>
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// </summary>

	void Room::InitializeGameRoom(const RoomAsset& roomAsset)
	{
		if (initialized)
			return;

		ENABLE_CALLS_ON_TICK();

		CreateEntityFreeList();
		if (twoPlayerGame)
		{
			multiplayerState->tickCounter = 1;

			// @NOTE: Ensure that entity id's line up
			if (multiplayerState->playerNumber == PlayerNumber::ONE)
			{
				CreateHostTank(Vec3f(0.0f, 0.1f, 0.0f));
				CreatePeerTank(Vec3f(0.0f, 0.1f, 0.0f));
			}
			else if (multiplayerState->playerNumber == PlayerNumber::TWO)
			{
				CreatePeerTank(Vec3f(0.0f, 0.1f, 0.0f));
				CreateHostTank(Vec3f(0.0f, 0.1f, 0.0f));
			}
		}
		else
		{
			CreateHostTank(Vec3f(0.0f, 0.1f, 0.0f));
		}

		if (1)
		{
			SpawnEnemyTank(Vec3f(7.0f, 0.1f, 7.0f)).SetNetworkOwner(PlayerNumber::ONE);
			SpawnEnemyTank(Vec3f(-7.0f, 0.1f, -7.0f)).SetNetworkOwner(PlayerNumber::ONE);
			SpawnEnemyTank(Vec3f(7.0f, 0.1f, -7.0f)).SetNetworkOwner(PlayerNumber::ONE);
			SpawnEnemyTank(Vec3f(-7.0f, 0.1f, 7.0f)).SetNetworkOwner(PlayerNumber::ONE);
		}

		Entity prop = CreateEntity("Prop01");
		prop.SetNetworkOwner(PlayerNumber::ONE);
		prop.SetRendering("alien", "Prop_01_DefaultMaterial_BaseColor");
		prop.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(2)));

		grid.Initialize(&roomAsset.map);
		CreateEntitiesFromGripMap();

		initialized = true;
		DISABLE_CALLS_ON_TICK();
	}

	void Room::UpdateGameRoom(real32 dt)
	{
		//if (!initialized && multiplayerState->IsValid())
		//	InitializeGameRoom();

		if (twoPlayerGame)
		{
			GameCommands* gameCommands = multiplayerState->GetNextGameCommands(this, dt);
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
			if (net->playerOwner == GetOppositePlayer(multiplayerState->playerNumber)
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
			if (bc->enabled && net->playerOwner == multiplayerState->playerNumber && !net->markedDestroyed)
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

		//DEBUGDrawAllColliders();
	}

	void Room::ShutdownGameRoom()
	{

	}

	/// <summary>
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// /// //////////////////////////////////////////////////////////////////////////////////////////////////////
	/// </summary>

	void Room::Initialize(const RoomAsset& roomAsset, MultiplayerState* multiplayerState)
	{
		this->type = roomAsset.type;
		this->multiplayerState = multiplayerState;
		this->name = roomAsset.name;

		switch (type)
		{
		case RoomType::MAIN_MENU: InitializeMenuRoom(); break;
		case RoomType::LEVEL_SELECTION: InitializeLevelSelectionRoom(); break;
		case RoomType::MULTIPLAYER: InitializeMultiplayerRoom(); break;
		case RoomType::LEVEL_1:
		{
			twoPlayerGame = 0;
			if (!twoPlayerGame)
			{
				multiplayerState->playerNumber = PlayerNumber::ONE;
			}

			InitializeGameRoom(roomAsset);
		}break;
		}
	}

	void Room::Update(real32 dt)
	{
		totalTime += dt;
		switch (type)
		{
		case RoomType::MAIN_MENU:  UpdateMenuRoom(dt); break;
		case RoomType::LEVEL_SELECTION: UpdateLevelSelectionRoom(dt); break;
		case RoomType::MULTIPLAYER: UpdateMultiplayerRoom(dt); break;
		case RoomType::LEVEL_1: UpdateGameRoom(dt); break;
		}
	}

	void Room::Shutdown()
	{
		switch (type)
		{
		case RoomType::MAIN_MENU:  ShutdownMenuRoom(); break;
		case RoomType::LEVEL_SELECTION: ShutdownLevelSelectionRoom(); break;
		case RoomType::MULTIPLAYER: ShutdownMultiplayeRoom(); break;
		case RoomType::LEVEL_1: ShutdownGameRoom(); break;
		}
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

		renderGroup->uiState = uiState;
	}

	void Room::DEBUGEnableTickCalls()
	{
		ENABLE_CALLS_ON_TICK();
	}

	void Room::DEBUGDisableTickCalls()
	{
		DISABLE_CALLS_ON_TICK();
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


	GridCell* Grid::GetCellFromPosition(const Vec3f& position)
	{
		Vec3f p = (position - Vec3f(topLeft.x, 0.0f, topLeft.y)) / CELL_EXTENT;

		int32 xIndex = (int32)(p.x);
		int32 yIndex = (int32)(p.z);

		if (IsValidIndex(xIndex, yIndex)) {
			int32 index = IndexOf2DArray(HORIZONTAL_CELL_COUNT, xIndex, yIndex);

			return &cells[index];
		}

		return nullptr;
	}

	bool Grid::IsValidIndex(int32 xIndex, int32 yIndex)
	{
		return (xIndex < HORIZONTAL_CELL_COUNT) && (xIndex >= 0) &&
			(yIndex < VERTICAL_CELL_COUNT) && (yIndex >= 0);
	}

	void Grid::Initialize(const FixedArray<int32, HORIZONTAL_CELL_COUNT* VERTICAL_CELL_COUNT>* map)
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
				cell.type = (GridCellType::Value)(*map->Get(index));
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

		//for (uint32 i = 0; i < cells.GetCapcity(); i++)
		//{
		//	Vec3f p1 = Vec3f(cells[i].position.x, h, cells[i].position.y);
		//	Vec3f p2 = p1 + Vec3f(0, 1, 0);
		//	Debug::DrawLine(p1, p2);
		//}
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
		Vec3f forwardDir = QuatToBasis(tankTransform.orientation).forward;
		tankTransform.position += Vec3f(moveDelta * forwardDir.x, 0.0f, moveDelta * forwardDir.z);

		// @NOTE: Collision detection 
		Sphere tankCollider = CreateSphere(tankTransform.position, 0.8f);
		for (uint32 i = 0; i < room->grid.cells.count; i++)
		{
			GridCell* cell = &room->grid.cells[i];
			if (cell->type == GridCellType::Value::WALL)
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
			PlaySound("gun_shotgun_cock_01", false);
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
				PlaySound("gun_rifle_dry_fire_01", false);
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
			if (cell->type == GridCellType::Value::WALL)
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
		Sphere collider = tank.GetSphereColliderWorld();
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
			else if (ent.GetTag() == Tag::Value::BULLET)
			{
				if (CheckIntersectionSphere(ent.GetSphereColliderWorld(), collider))
				{
					PlaySound("explosion_large_01", false);
					room->GameCommandDestroyEntity(tank);
					room->GameCommandDestroyEntity(bulletSpawnPoint);
					room->GameCommandDestroyEntity(entity);
					room->GameCommandDestroyEntity(ent);

					return;
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

	void GameState::TransitionToRoom(const RoomAsset& roomAsset)
	{
		currentRoom.Shutdown();
		ZeroStruct(&currentRoom);

		multiplayerState.tickCounter = 0;
		multiplayerState.gatherCommands = 0;
		multiplayerState.timeSinceLastSend = 0;
		ZeroStruct(&multiplayerState.lastCommands);
		ZeroStruct(&multiplayerState.currentCommands);

		currentRoom.Initialize(roomAsset, &multiplayerState);
		nextRoom = RoomType::INVALID;
	}

	bool32 Game::Initialize()
	{
		GameState::Initialize(GameMemory::PushPermanentStruct<GameState>());
		GetGameState();
		gs->nextRoom = RoomType::MAIN_MENU;

		return true;
	}

	void Game::UpdateGame(real32 dt)
	{
		GetGameState();
		if (gs->nextRoom != RoomType::INVALID)
		{
			RoomAsset* testRoom = GameMemory::PushTransientStruct<RoomAsset>();

			if (gs->nextRoom == RoomType::LEVEL_1)
			{
				GetAssetState();
				testRoom = as->rooms.Get(0);
			}

			testRoom->type = gs->nextRoom;
			gs->TransitionToRoom(*testRoom);
		}
		else
		{
			gs->currentRoom.Update(dt);
		}
	}

	void Game::ConstructRenderGroup(EntityRenderGroup* renderGroup)
	{
		GetGameState();
		gs->currentRoom.ConstructRenderGroup(renderGroup);
	}

	void Game::Shutdown()
	{

	}

	void UserInterfaceState::Start()
	{
		uiElements.Clear();
	}

	void UserInterfaceState::Rect(real32 oX, real32 oY, real32 halfWidth, real32 halfHeight, const Vec4f& colour)
	{
		UIElement el = {};
		el.type = UIElementType::RECT;
		el.rect.oX = oX;
		el.rect.oY = oY;
		el.rect.width = halfWidth * 2.0f;
		el.rect.height = halfHeight * 2.0f;
		el.rect.texture = 0;
		el.rect.colour = colour;

		uiElements.Add(el);
	}

	void UserInterfaceState::Rect(real32 oX, real32 oY, real32 halfWidth, real32 halfHeight, AssetId textureId, const Vec4f& colour)
	{
		UIElement el = {};
		el.type = UIElementType::RECT;
		el.rect.oX = oX;
		el.rect.oY = oY;
		el.rect.width = halfWidth * 2.0f;
		el.rect.height = halfHeight * 2.0f;
		el.rect.texture = textureId;
		el.rect.colour = colour;

		uiElements.Add(el);
	}

	void UserInterfaceState::Text(const CString& text, real32 oX, real32 oY, real32 scale)
	{
		UIText tex = {};
		tex.text = text;
		tex.oX = oX;
		tex.oY = oY;
		tex.scale = scale;


		UIElement el = {};
		el.type = UIElementType::TEXT;
		el.text = tex;
		uiElements.Add(el);
	}

	bool UserInterfaceState::Button(const CString& text, real32 oX, real32 oY, real32 width, real32 height, real32 scale)
	{
		Vec2f min = Vec2f(oX - width, oY - height);
		Vec2f max = Vec2f(oX + width, oY + height);

		bool hovered = false;
		bool clicked = false;

		GetInput();
		if (input->mouse_norm.x > min.x &&
			input->mouse_norm.y > min.y &&
			input->mouse_norm.x < max.x &&
			input->mouse_norm.y < max.y)
		{
			hovered = true;
			if (IsKeyJustDown(input, mb1))
			{
				clicked = true;
			}
		}

		{
			UIRect rect = {};
			rect.oX = oX;
			rect.oY = oY;
			rect.width = hovered ? width * 1.01f : width;
			rect.height = hovered ? height * 1.01f : height;
			rect.colour = Vec4f(0.5f, 0.5f, 0.5f, 1.0f);

			UIElement el = {};
			el.type = UIElementType::RECT;
			el.rect = rect;
			uiElements.Add(el);
		}

		{
			UIText tex = {};
			tex.text = text;
			tex.oX = oX;
			tex.oY = oY + height / 4.0f;
			tex.scale = scale;

			UIElement el = {};
			el.type = UIElementType::TEXT;
			el.text = tex;
			uiElements.Add(el);
		}

		return clicked;
	}

	void UserInterfaceState::End()
	{
	}
}
