#include "SolarGameTypes.h"
#include "core/SolarEvent.h"

namespace sol
{
	static Room* currentRoom = nullptr;

	bool8 Room::Initliaze(RoomResource* res)
	{
		currentRoom = this;

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
		entity->room = this;

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

		//CHECK_CALLER_IS_ON_TICK();

		if (entity && entity->IsValid())
		{
			//LOG("Destorying entity: " << entity.id.index);
			RemoveEntityChildParentRelationship(entity);
			PushFreeEntityId(entity->id);

			int32 index = entity->id.index;
			GameMemory::ZeroStruct(&entities[index]);
			GameMemory::ZeroStruct(&nameComponents[index]);
			GameMemory::ZeroStruct(&tagComponents[index]);
			GameMemory::ZeroStruct(&materialComponets[index]);
			//ZeroStruct(&colliderComponents[index]);

			// @NOTE: We don't zero the brain beacause if the brain is calling this function it will
			//		: mess the current state of the brain, and thus any futher code is be broken entirely
			//		: instead what happens is the brain is zero'd upon being set.
			//brainComponents[index].enabled = false;
			//ZeroStruct(&brainComponents[index]);

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