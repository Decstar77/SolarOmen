#include "TankGame.h"
#include "core/SolarCore.h"
#include "core/SolarGame.h"
#include "ManifoldTests.h"

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

		return *entity;
	}

	Entity Room::CreateEntity(const CString& name)
	{
		Entity entity = CreateEntity();
		entity.SetName(name);

		return entity;
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

	void Room::Initialize()
	{
		GetPlatofrmState();
		CreateEntityFreeList();

		tank = CreateEntity("Tank");
		tank.SetModel("Tank");
		tank.SetTexture("Tank_DefaultMaterial_BaseColor");
		tank.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(0.65f)));
		tank.EnableCollider();
		tank.SetCollider(CreateAABBFromCenterRadius(Vec3f(0, 0, 0), Vec3f(1, 1, 1)));

		player2Tank = CreateEntity("Tank2");
		player2Tank.SetModel("Tank2");
		player2Tank.SetTexture("Tank_DefaultMaterial_BaseColor");
		player2Tank.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(0.65f)));
		player2Tank.EnableCollider();
		player2Tank.SetCollider(CreateAABBFromCenterRadius(Vec3f(0, 0, 0), Vec3f(1, 1, 1)));

		Entity prop = CreateEntity("Prop01");
		prop.EnableRendering();
		prop.SetModel("Prop_01");
		prop.SetTexture("Prop_01_DefaultMaterial_BaseColor");
		prop.SetLocalTransform(Transform(Vec3f(0, 0, 0), Quatf(), Vec3f(1)));

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
				cell->entity = entity;
			}
		}

		//PlaySound("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Audio/Monsters.wav", true);
	}

	void Room::Update(real32 dt)
	{
		GetInput();

		Transform tankTransform = tank.GetWorldTransform();

		if (input->a)
		{
			tankTransform.position.x -= 3.0f * dt;
		}
		if (input->d)
		{
			tankTransform.position.x += 3.0f * dt;

		}
		if (input->w)
		{
			tankTransform.position.z += 3.0f * dt;
		}
		if (input->s)
		{
			tankTransform.position.z -= 3.0f * dt;
		}

		AABB tankCollider = tank.GetAlignedBoxColliderLocal();
		tankCollider = UpdateAABB(tankCollider, tankTransform.position, tankTransform.orientation, tankTransform.scale);
		for (uint32 i = 0; i < grid.cells.count; i++)
		{
			GridCell* cell = &grid.cells[i];
			if (cell->occupied == 1)
			{
				AABB aabb = cell->entity.GetAlignedBoxColliderWorld();
				Manifold manifold = {};
				if (CheckManifoldAABB(tankCollider, aabb, &manifold))
				{
					Vec3f disp = manifold.normal * manifold.seperationDistance;
					tankTransform.position += disp;
					tankCollider = AABBSetPosition(tankCollider, tankTransform.position);
				}

				//Debug::DrawAABB(aabb);
			}
		}

		//Debug::DrawAABB(tankCollider);
		tank.SetLocalTransform(tankTransform);
		playerCamera.transform.position = Lerp(playerCamera.transform.position, tankTransform.position + playerCameraOffset, 0.1f);



		uint8 networkBuffer[256] = {};
		while (Platform::NetworkReceive(networkBuffer, ArrayCount(networkBuffer)) >= 0)
		{
			if (networkBuffer[0] > 1)
			{
				SnapShotType type = (SnapShotType)networkBuffer[0];
				switch (type)
				{
				case SnapShotType::TRANSFORM:
				{
					SnapShotTransform snapTransform = *((SnapShotTransform*)networkBuffer);
					Transform t = player2Tank.GetWorldTransform();
					t.position = Lerp(snapTransform.position, t.position, 0.01f);
					player2Tank.SetLocalTransform(t);
				}break;
				}
			}

			ZeroArray(networkBuffer);
		}

		if (Platform::NetworkConnectionEsablished())
		{
			static real32 nextSend = 0.0f;
			nextSend += dt;
			// @NOTE: 30 packet per second
			if (nextSend >= 0.033f)
			{
				SnapShotTransform snap = {};
				snap.type = SnapShotType::TRANSFORM;
				snap.position = tank.GetWorldTransform().position;
				Platform::NetworkSend(&snap, sizeof(snap));
				nextSend = 0;
			}
		}


		//for (int32 i = 0; i < ENTITY_STORAGE_COUNT; i++)
		//{
		//	ColliderComponent* cc = &colliderComponents[i];
		//	TransformComponent* tr = &transformComponents[i];

		//	if (cc->enabled)
		//	{
		//		switch (cc->type)
		//		{
		//		case ColliderType::SPHERE:
		//		{
		//			Sphere sphere = TranslateSphere(cc->sphere, tr->transform.position);
		//			//Debug::DrawSphere(sphere);
		//		}
		//		break;
		//		case ColliderType::ALIGNED_BOUNDING_BOX:
		//		{
		//			AABB box = UpdateAABB(cc->alignedBox, tr->transform.position, tr->transform.orientation, tr->transform.scale);
		//			Debug::DrawAABB(box);
		//		}
		//		break;
		//		case ColliderType::BOUDNING_BOX:
		//		{
		//		}
		//		break;
		//		}
		//	}
		//}

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

}
