#pragma once

#include "../core/SolarApplication.h"
#include "../renderer/RendererTypes.h"
#include "../resources/SolarResourceTypes.h"
#include "../core/SolarPrimitives.h"

namespace sol
{
	struct EntityId
	{
		int32 index;
		int32 generation;

		class Entity* Get() const;
		inline String ToString() const { return String("Index;").Add(index).Add(":Gen;").Add(generation); };

		inline bool operator==(const EntityId& rhs) const { return this->index == rhs.index && this->generation == rhs.generation; }
		inline bool operator!=(const EntityId& rhs) const { return this->index != rhs.index || this->generation != rhs.generation; }
	};


	struct TransformComponent
	{
		Transform transform;
	};

	struct NameComponent
	{
		String name;
	};

	struct TagComponent
	{
		uint32 tag;
	};

	struct MaterialComponent
	{
		Material material;
	};

	class Entity
	{
	public:
		SOL_API String GetName() const;
		SOL_API void SetName(const String& name);
		SOL_API EntityId GetId() const;
		SOL_API bool IsValid() const;

		SOL_API void SetParent(Entity* entity);
		SOL_API Entity* GetParent();
		SOL_API Entity* GetFirstChild();
		SOL_API Entity* GetSiblingAhead();
		SOL_API Entity* GetSiblingBehind();
		SOL_API ManagedArray<Entity*> GetChildren();

		SOL_API void SetLocalTransform(const Transform& transform);
		SOL_API Transform GetLocalTransform() const;
		SOL_API Transform GetWorldTransform() const;

		SOL_API MaterialComponent* GetMaterialomponent();
		SOL_API void SetMaterial(const String& modelName, const String& textureName);

		SOL_API operator bool() const;
		SOL_API bool operator==(const Entity& rhs) const;
		SOL_API bool operator!=(const Entity& rhs) const;

	private:
		EntityId id;
		EntityId parent;
		EntityId child;
		EntityId siblingAhead;
		EntityId siblingBehind;
		class Room* room;

	private:
		friend struct GameState;
		friend struct EntityId;
		friend class Room;
	};

	struct Camera
	{
		Transform transform;

		// @NOTE: Temporary
		real32 pitch;
		real32 yaw;

		real32 far_;
		real32 near_;
		real32 yfov;
		real32 aspect;

		SOL_API Ray ShootRayAtMouse()const;

		inline Mat4f GetViewMatrix() const { return Inverse(transform.CalculateTransformMatrix()); }
		inline Mat4f GetProjectionMatrix() const { return PerspectiveLH(DegToRad(yfov), aspect, near_, far_); }
	};

	class Room
	{
	public:
		static constexpr uint32 ENTITY_STORAGE_COUNT = 1000;
		static constexpr uint32 INVALID_ENTITY_INDEX = 0;

		ResourceId id;
		String name;

		bool8 twoPlayerGame;
		bool8 initialized;
		bool8 isPaused;

		Camera camera;

		FixedArray<Entity, ENTITY_STORAGE_COUNT>				entities;
		FixedArray<EntityId, ENTITY_STORAGE_COUNT - 1>			entityFreeList;

		FixedArray<TransformComponent, ENTITY_STORAGE_COUNT>	transformComponents;
		FixedArray<NameComponent, ENTITY_STORAGE_COUNT>			nameComponents;
		FixedArray<TagComponent, ENTITY_STORAGE_COUNT>			tagComponents;
		FixedArray<MaterialComponent, ENTITY_STORAGE_COUNT>		materialComponets;

		//FixedArray<ColliderComponent, ENTITY_STORAGE_COUNT> colliderComponents;
		//FixedArray<BrainComponent, ENTITY_STORAGE_COUNT> brainComponents;
		//FixedArray<NetworkComponent, ENTITY_STORAGE_COUNT> networkComponents;
	public:
		SOL_API bool8 Initliaze(RoomResource* res);

		SOL_API Entity CreateEntity();
		SOL_API Entity CreateEntity(const String& name);
		SOL_API void DestoryEntity(Entity* entity);

		SOL_API void BeginEntityLoop();
		SOL_API Entity GetNextEntity();

		SOL_API void ContructRenderPacket(RenderPacket* renderPacket);
		SOL_API void ContructResource(RoomResource* res);

	private:
		uint32 entityLoopIndex;

		void CreateEntityFreeList();
		EntityId GetNextFreeEntityId();
		void PushFreeEntityId(EntityId id);
		void RemoveEntityChildParentRelationship(Entity* entity);
	};

	struct Game
	{
		ApplicationConfigs appConfig;
		bool8(*Initialize)(Game* game);
		bool8(*Update)(Game* game, RenderPacket* renderPacket, real32 dt);
		void(*Shutdown)(Game* game);
	};

	extern bool8 CreateGame(Game* game);
}