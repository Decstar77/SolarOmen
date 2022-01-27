#include "SolarGameTypes.h"
#include "core/SolarEvent.h"

namespace sol
{
	static Room* currentRoom = nullptr;

	bool8 Room::Initliaze(RoomResource* res)
	{
		currentRoom = this;
		Entity::room = this;

		CreateEntityFreeList();

		camera = {};
		camera.far_ = 100.0f;
		camera.near_ = 0.3f;
		camera.yfov = 45.0f;
		camera.aspect = Application::GetSurfaceAspectRatio();
		camera.transform = Transform();
		camera.transform.position.x = 7;
		camera.transform.position.y = 14;
		camera.transform.position.z = -7;
		camera.transform.LookAtLH(0);

		isPaused = false;
		initialized = true;

		if (res)
		{
			id = res->id;
			name = res->name;
			skyboxId = res->skyBoxId;
		}

		return 1;
	}

	Entity* EntityId::Get() const
	{
		if (index > 0 && index < (int32)currentRoom->entities.GetCapcity())
		{
			Entity* stored = &currentRoom->entities[index];
			if (stored->id == *this)
			{
				return stored;
			}
		}

		return nullptr;
	}

	Entity Room::CreateEntity()
	{
		EntityId id = GetNextFreeEntityId();
		Assert(id.index > 0 && id.index < (int32)entities.GetCapcity(), "Entity id index was invalid");

		Entity* entity = &entities[id.index];
		Assert(entity->id.index == INVALID_ENTITY_INDEX, "Using a valid entity");
		entity->id = id;

		transformComponents[id.index].transform = Transform();

		//BrainComponent* bc = &brainComponents[id.index];
		//ZeroStruct(bc);

		//NetworkComponent* nc = &networkComponents[id.index];
		//ZeroStruct(nc);

		return *entity;
	}

	Entity Room::CreateEntity(const String& name)
	{
		Entity entity = CreateEntity();
		entity.SetName(name);

		return entity;
	}

	void Room::DestoryEntity(Entity* entity)
	{
		Assert(entity, "Destroy null entity ?");

		//CHECK_CALLER_IS_ON_TICK();

		if (entity && entity->IsValid())
		{
			int32 index = entity->id.index;

			// @NOTE: Remove children
			TransformComponent* transformComp = &transformComponents[index];
			for (uint32 i = 0; i < transformComp->children.count; i++)
			{
				Entity child = {};
				child.id = transformComp->children[i];
				DestoryEntity(&child);
			}

			if (transformComp->parent.index > 0) {
				transformComponents[transformComp->parent.index].children.Remove(entity->id);
			}


			PushFreeEntityId(entity->id);
			GameMemory::ZeroStruct(&transformComponents[index]);
			GameMemory::ZeroStruct(&entities[index]);
			GameMemory::ZeroStruct(&nameComponents[index]);
			GameMemory::ZeroStruct(&tagComponents[index]);
			GameMemory::ZeroStruct(&materialComponets[index]);

			*entity = {};
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

			if (entityLoopIndex == entities.count)
			{
				return entity;
			}
		}

		return {};
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
			GameMemory::ZeroStruct(entities.Get(i));
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

	void Room::ContructRenderPacket(RenderPacket* renderPacket)
	{
		renderPacket->viewMatrix = camera.GetViewMatrix();
		renderPacket->projectionMatrix = camera.GetProjectionMatrix();
		renderPacket->skyboxId = skyboxId;

		BeginEntityLoop();
		while (Entity entity = GetNextEntity())
		{
			RenderEntry entry = {};
			entry.worldTransform = entity.GetWorldTransform();
			entry.material = entity.GetMaterialomponent()->material;

			renderPacket->renderEntries.Add(entry);
		}
	}

	void Room::ContructResource(RoomResource* res)
	{
		res->id = id;
		res->name = name;
		res->skyBoxId = skyboxId;
	}
}