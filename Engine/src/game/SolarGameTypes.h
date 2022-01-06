#pragma once

#include "../core/SolarApplication.h"
#include "../renderer/RendererTypes.h"
#include "../resources/SolarResourceTypes.h"

namespace sol
{
	struct SOL_API EntityId
	{
		int32 index;
		int32 generation;

		class Entity* Get() const;
		inline String ToString() const { return String("Index;").Add(index).Add(":Gen;").Add(generation); };

		inline bool operator==(const EntityId& rhs) const { return this->index == rhs.index && this->generation == rhs.generation; }
		inline bool operator!=(const EntityId& rhs) const { return this->index != rhs.index || this->generation != rhs.generation; }
	};

	class SOL_API Entity
	{
	public:
		String GetName();
		void SetName(const String& name);

		operator bool() const;
		bool operator==(const Entity& rhs) const;
		bool operator!=(const Entity& rhs) const;

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

	class SOL_API Room
	{
	public:
		static constexpr uint32 ENTITY_STORAGE_COUNT = 1000;
		static constexpr uint32 INVALID_ENTITY_INDEX = 0;

		ResourceId id;
		String name;

		bool8 twoPlayerGame;
		bool8 initialized;
		bool8 isPaused;

		FixedArray<Entity, ENTITY_STORAGE_COUNT> entities;
		FixedArray<EntityId, ENTITY_STORAGE_COUNT - 1> entityFreeList;

	public:
		Entity CreateEntity();
		Entity CreateEntity(const String& name);
		void DestoryEntity(Entity* entity);

		void BeginEntityLoop();
		Entity GetNextEntity();

	private:
		uint32 entityLoopIndex;

		void CreateEntityFreeList();
		EntityId GetNextFreeEntityId();
		void PushFreeEntityId(EntityId id);
		void RemoveEntityChildParentRelationship(Entity* entity);
	};

	struct SOL_API Camera
	{
		Transform transform;

		// @NOTE: Temporary
		real32 pitch;
		real32 yaw;

		real32 far_;
		real32 near_;
		real32 yfov;
		real32 aspect;

		inline Mat4f GetViewMatrix() const { return Inverse(transform.CalculateTransformMatrix()); }
		inline Mat4f GetProjectionMatrix() const { return PerspectiveLH(DegToRad(yfov), aspect, near_, far_); }
	};

	struct SOL_API Game
	{
		ApplicationConfigs appConfig;
		bool8(*Initialize)(Game* game);
		bool8(*Update)(Game* game, RenderPacket* renderPacket, real32 dt);
		void(*Shutdown)(Game* game);
	};

	extern bool8 CreateGame(Game* game);
}