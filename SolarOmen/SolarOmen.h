#pragma once
#include "core/SolarPlatform.h"
#include "SolarAudio.h"
#include "FileParsers.h"
#include "SolarAssets.h"
#include "core/SolarMemory.h"
#include "Physics/SolarPhysics.h"
#include "SimpleColliders.h"
#include "ManifoldTests.h"

#include "components/SolarCamera.h"
#include "components/SolarLight.h"

namespace cm
{
	struct EntityId
	{
		int32 index;
		int32 generation;

		struct Entity* Get();
		CString ToString();

		inline bool operator==(const EntityId& rhs) const
		{
			return this->index == rhs.index && this->generation == rhs.generation;
		}

		inline bool operator!=(const EntityId& rhs) const
		{
			return this->index != rhs.index || this->generation != rhs.generation;
		}
	};

	enum class EntityType
	{
		ENVIRONMENT = 1,
		UNIT,
		PARTICLE_EMITTER,
		ENTITY_TYPE_COUNT
	};

	inline std::string EntityTypeToString(const EntityType& type)
	{
		switch (type)
		{
		case EntityType::ENVIRONMENT: return "Environment";
		case EntityType::PARTICLE_EMITTER: return "Particle emitter";
		case EntityType::UNIT: return "UNIT";
		default: return "Unkown entity type";
		}
	}

#define ENTITY_TYPE_ELIF(type) else if (str == #type) return EntityType::##type

	inline EntityType StringToEntityType(const CString& str)
	{
		if (str == "ENVIRONMENT")
		{
			return EntityType::ENVIRONMENT;
		}
		ENTITY_TYPE_ELIF(UNIT);

		LOG("Unkown entity type");
		return  EntityType::ENVIRONMENT;
	}



	enum class RenderFlag : uint32
	{
		NONE = 0,
		REQUIRES_ALPHA_TESTING = SetABit(1),
		NO_CAST_SHADOW = SetABit(2),
		NO_RECEIVES_SHADOW = SetABit(3)
	};

	enum class ColliderType
	{
		SPHERE,
		BOX,
		MESH
	};

	class CollisionComponent
	{
	public:
		bool32 active;
		ColliderType type;

		union
		{
			Sphere sphere;
			OBB box;
			int32 meshIndex;
		};

		AABB GetBoundingBox();

		CollisionComponent() {}
	};

	struct Material
	{
		ShaderId shaderId;
		TextureId albedoTex;
		TextureId occRoughMetTex;
		TextureId normalTex;
		TextureId emissiveTex;
	};

	struct RenderComponent
	{
		bool32 active;
		uint32 flags;

		ModelId modelId;
		Material material;

		inline void SetFlag(const RenderFlag& flag)
		{
			flags = flags | (uint32)flag;
		}

		inline bool32 HasFlagSet(const RenderFlag& flag)
		{
			return flags & (uint32)flag;
		}
	};



	class BrainNode
	{
	};

	class BrainNodeTransform
	{
		void Run(Vec3f* pos, Quatf* ori, Vec3f* scl);
	};

	class BrainNodeOther
	{
		void Run(LightComponent* light);
	};

	struct BrainComponent
	{
		bool32 active;
		int32 tableIndex;
	};

	struct PlayerPart
	{
		real32 pitch;
		real32 yaw;
		real32 soundStepInterval;
		bool32 isSprinting;
		Vec3f acceleration;
		Vec3f velocity;
		bool32 grounded;
		real32 groundCheckDist;
		Ray groundRay;
		Sphere collider;
	};

	enum class EntityFlag
	{
		SIMULATE_PHYSICS = SetABit(1)
	};



#define MAX_PARTICLE_EMITTERS 10
#define MAX_PARTICLES_PER_EMITTER 1000

	struct Particle
	{
		int32 index;
		int32 emiterSlot;
		real32 currentLife;

		Vec3f velocity;
		Vec3f acceleration;
		Vec3f angularAccelertation;

		Vec3f colour;
		Transform transform;

		inline bool IsAlive() { return currentLife > 0; }
	};

	enum class ParticleEmitterType
	{
		Fire,
	};

	struct ParticleEmitterPart
	{
		int32 particleCount;
		int32 emitterSlot;
		Particle* particles;

		int32 maxLife;
		real32 timeSeconds;
		real32 spawnRateSeconds;

		Vec3f startingColour;
		Vec3f endingColour;

		real32 startingSize;
		real32 endingSize;

		Vec3f startingVelocity;
		Vec3f acceleration;

		int32 meshId;
	};

	struct TransformComponent
	{
		EntityId id;
	};

	class CarComponent
	{
	public:
		bool32 active;
		real32 speed;
		real32 follow;
		int32 laneIndex;
	};

	class RacingWaypoint
	{
	public:
		bool32 active;
		int32 laneIndex;
		int32 waypointIndex;
	};

	struct Entity
	{
		bool32 active;
		Transform transform;
		AABB boundingBoxLocal;

		CollisionComponent collisionComp;
		RigidBodyComponent rigidBody;
		RenderComponent renderComp;
		LightComponent lightComp;
		CameraComponent cameraComp;
		CarComponent carComp;

		EntityType type;
		union
		{
			PlayerPart playerPart;
			ParticleEmitterPart particlePart;
		};

		CString* GetName();
		void SetName(const CString& name);

		EntityId GetId();
		bool32 IsValid();
		bool32 IsSameEntity(Entity* other);
		bool32 IsPhsyicsEnabled();
		AABB GetWorldBoundingBox();

		Entity* GetParent();
		Entity* GetFirstChild();
		Entity* GetSibling();
		ManagedArray<Entity*> GetChildren();
		void SetParent(EntityId entity);

		Transform GetLocalTransform();
		Transform GetWorldTransform();

		void SetCollider(const Sphere& sphere, bool32 setActive = true);
		void SetCollider(const OBB& box, bool32 setActive = true);
		void SetCollider(const ModelId& mesh, bool32 setActive = true);

		RacingWaypoint* GetRacingWaypointComponent();


	private:
		EntityId id;
		EntityId parent;
		EntityId child;
		EntityId sibling;

		EntityFlag flags;

		GameState* gs;

	private:
		friend class GameState;
		friend struct EntityId;

		Entity()
		{
			// @NOTE: All handeled in game create entity function
		}

	};


	//	inline OBB GetEntityBoxCollider(Entity* entity)
	//	{
	//		OBB result = {};
	//#if 0
	//		if (index >= 0 && index < ArrayCount(entity->colliders))
	//		{
	//			OBB base = entity->colliders[index];
	//			result.center = base.center + entity->transform.position;
	//			result.basis.mat = entity->transform.GetBasis().mat * result.basis.mat;
	//			result.extents = base.extents * entity->transform.scale;
	//		}
	//#else
	//		Mat4f base = entity->collision.box.mat; base[3][3] = 1.0f;
	//		Mat4f delta = entity->transform.CalculateTransformMatrix();
	//
	//		Mat4f world = base * delta;
	//		result.center = Vec3f(world.row3);
	//		result.basis = RemoveScaleFromRotationMatrix(Mat3f(world));
	//		result.extents = entity->collision.box.extents * entity->transform.scale;
	//#endif
	//		return result;
	//	}

	////////////////////////////////////////////////
	// @NOTE: Entity stuff
	////////////////////////////////////////////////



#define INVALID_INDEX 0
#define COLLISION_CELL_DIMS 1

	struct CollisionCell
	{
		int32 xIndex;
		int32 zIndex;
		bool32 filled;
		Vec3f world_position;
	};

#define COLLISION_GRID_WIDTH 32
#define COLLISION_GRID_DEPTH 32

	struct CollisionGrid
	{
		Vec3f base_position;
		CollisionCell cells[COLLISION_GRID_WIDTH][COLLISION_GRID_DEPTH];
	};

	inline AABB GetCollisionCellBox(CollisionGrid* grid, CollisionCell* cell, real32 height = 5.0f)
	{
		Vec3f min = grid->base_position + cell->world_position;
		Vec3f max = grid->base_position + cell->world_position + Vec3f(COLLISION_CELL_DIMS, height, COLLISION_CELL_DIMS);

		Vec3f center = (min + max) / 2.0f;
		Vec3f radius = (max - min) / 2.0f;

		AABB result = CreateAABBFromCenterRadius(center, radius * 0.98f);

		return result;
	}

	struct WorldSector
	{
		int32 neighbour1;
		int32 neighbour2;
		AABB boundingBox;
	};



	// @NOTE: Also in shader !!
#define MAX_DIRECTIONAL_LIGHT_COUNT 2
#define MAX_SPOT_LIGHT_COUNT 8
#define MAX_POINT_LIGHT_COUNT 16

	struct OpaqueRenderEntry
	{
		// @NOTE: Transform is in world space
		Transform transform;
		RenderComponent renderComp;
	};

	struct EntityRenderGroup
	{
		Camera mainCamera;
		Camera playerCamera;

		Entity player;

		int32 pointLightCount;
		Entity pointLights[MAX_POINT_LIGHT_COUNT];

		Entity mainDirectionalLight;

		int32 particleEmitterCount;
		Entity particleEmitters[MAX_PARTICLE_EMITTERS];

		int32 opaqueEntityCount;
		OpaqueRenderEntry opaqueRenderEntries[128];

		int32 testingCount;
		Entity testingEntities[128];

		inline void ClearRenderGroup()
		{
			ZeroStruct(this);
		}
	};

	struct TransientState
	{
		// @NOTE: The current renderable entities visable to the game camera
		EntityRenderGroup render_group;

		int32 rigidBodyCount;
		Entity* rigidBodies[1024];

		int32 collisionManifoldCount;
		Manifold collisionManifolds[1024];

		PhysicsSimulator physicsSimulator;
	};




#define ENTITY_STORAGE_COUNT 1000
	class GameState
	{
	public:
		Entity* player;
		Entity* gun;
		Entity* playerCamera;

		int32 entityCount;
		int32 entityLoopIndex;
		int32 entityLoopCount;
		Entity entites[ENTITY_STORAGE_COUNT];

		int32 entityFreeListCount;
		EntityId entityFreeList[ENTITY_STORAGE_COUNT - 1];


		FixedArray<EntityId, ENTITY_STORAGE_COUNT> entityIds;
		FixedArray<CString, ENTITY_STORAGE_COUNT>  nameComponents;
		FixedArray<RacingWaypoint, ENTITY_STORAGE_COUNT> racingWaypointComponents;

		//FixedArray<LightComponent, ENTITY_STORAGE_COUNT> lightsComponents;

		int32 meshColliderCount;
		MeshCollider meshColliders[ENTITY_STORAGE_COUNT];



		int32 particleEmitterCount;
		int32 particleSlot[MAX_PARTICLE_EMITTERS];
		Particle partices[MAX_PARTICLE_EMITTERS * MAX_PARTICLES_PER_EMITTER];

		CollisionGrid collision_grid;

		int32 worldSectorCount;
		WorldSector worldSectors[100];

		Entity* testEntity1;
		Entity* testEntity2;


		Camera camera;
		Vec3f camera_offset_player;

		bool32 is_initialized;
		real32 dt;

	public:
		inline EntityId GetNextFreeEntityId()
		{
			entityFreeListCount--;
			int32 index = entityFreeListCount;
			Assert(entityFreeListCount > 0, "No more free ids");
			EntityId id = entityFreeList[index];

			return id;
		}

		inline Entity* CreateEntity()
		{
			EntityId id = GetNextFreeEntityId();
			Assert(id.index > 0 && id.index <= ArrayCount(entites), "Entity id index was invalid");

			Entity* entity = &entites[id.index];
			// @NOTE: This zero probably not nesscarry, but for safety
			ZeroStruct(entity);
			entity->id = id;
			entity->active = true;
			entity->gs = this;
			entity->transform = Transform();
			entity->type = EntityType::ENVIRONMENT;
			entity->SetName("No name brand");
			entityCount++;

			entityIds[id.index] = id;

			return entity;
		}

		inline void BeginEntityLoop()
		{
			entityLoopIndex = 0;
			entityLoopCount = 0;
		}

		inline Entity* GetNextEntity()
		{
			entityLoopIndex++;

			while (entityLoopCount < entityCount)
			{
				Entity* entity = &entites[entityLoopIndex];
				if (entity->IsValid())
				{
					entityLoopCount++;
					return entity;
				}
				entityLoopIndex++;
			}

			return nullptr;
		}

	public:
		static inline void Initialize(GameState* gs) { gameState = gs; }
		static inline GameState* Get() { return gameState; }

		void BuildTrackFromEntities();
		void DEBUGDrawCurrentTrack();

		inline void CreateEntityFreeList()
		{
			entityCount = 0;
			entityFreeListCount = ArrayCount(entityFreeList);
			for (int32 i = entityFreeListCount - 1; i >= 0; i--)
			{
				EntityId id = {};
				id.index = entityFreeListCount - i;
				id.generation = 0;
				entityFreeList[i] = id;
			}

			// @NOTE: Well this clear isn't stricly nessessary it's done for clarity sake
			for (int32 i = 0; i < ArrayCount(entites); i++)
			{
				ZeroStruct(&entites[i]);
			}
		}

		//inline void AddFreeEntityId(Entity* entity)
		//{
		//	EntityId id = entity->id;
		//	id.generation++;
		//	Assert(entityFreeListCount >= 0 &&
		//		entityFreeListCount < ArrayCount(entityFreeList), "Invalid free index");

		//	if (entityFreeListCount >= 0 &&
		//		entityFreeListCount < ArrayCount(entityFreeList))
		//	{
		//		entityFreeList[entityFreeListCount] = id;
		//		entityFreeListCount++;
		//	}
		//}

		//inline void RemoveEntity(Entity* entity)
		//{
		//	if (entity && entity->id.index)
		//	{
		//		AddFreeEntityId(entity);
		//		entityCount--;

		//		if (entity->type == EntityType::PARTICLE_EMITTER)
		//		{
		//			particleSlot[entity->particlePart.emitterSlot] = 0;
		//		}

		//		// @NOTE: This clear is not strictly nessessary but is done in an to prevent bugs in old code
		//		ZeroStruct(entity);
		//	}
		//}

		//inline void RemoveAllEntities()
		//{
		//	for (int32 i = 0; i < ArrayCount(entites); i++)
		//	{
		//		RemoveEntity(&entites[i]);
		//	}
		//}

	private:
		inline static GameState* gameState = nullptr;
	};

	inline bool32 CompareGameStates(GameState* a1, GameState* a2)
	{
		for (int32 i = 0; i < ArrayCount(a1->entites); i++)
		{
			if (!ByteCompareBetweenStructs(&a1->entites[i], &a2->entites[i]))
			{
				return false;
			}
		}

		return true;
	}


	void ConstructRenderGroup(GameState* gs, EntityRenderGroup* renderGroup);



	inline void CreateCollisionGrid(GameState* gs)
	{
		CollisionGrid* grid = &gs->collision_grid;
		for (int32 z = 0; z < COLLISION_GRID_DEPTH; z++)
		{
			for (int32 x = 0; x < COLLISION_GRID_WIDTH; x++)
			{
				CollisionCell cell = {};
				cell.xIndex = x;
				cell.zIndex = z;
				cell.world_position = Vec3f((real32)COLLISION_CELL_DIMS * x, 0.0f, (real32)COLLISION_CELL_DIMS * z);
				cell.filled = false;

				grid->cells[z][x] = cell;
			}
		}
		grid->base_position.x = -COLLISION_CELL_DIMS * COLLISION_GRID_WIDTH / 2;
		grid->base_position.z = -COLLISION_CELL_DIMS * COLLISION_GRID_DEPTH / 2;
	}

	inline WorldSector* CreateWorldSector(GameState* gs, Vec3f size = Vec3f(10))
	{
		Assert(gs->worldSectorCount < ArrayCount(gs->worldSectors), "To many sectors");

		int32 index = gs->worldSectorCount;
		WorldSector* sector = &gs->worldSectors[index];

		sector->boundingBox = CreateAABBFromCenterRadius(0, size);
		sector->neighbour1 = -1;
		sector->neighbour2 = -1;

		gs->worldSectorCount++;

		return sector;
	}

	inline bool32 IsEntityInWorldSector(Entity* entity, WorldSector* sector)
	{
		if (sector)
		{
			AABB entityBoundingBox = entity->GetWorldBoundingBox();

			if (CheckIntersectionAABB(entityBoundingBox, sector->boundingBox))
			{
				return true;
			}
		}

		return false;
	}

	void BakeCollisionGrid(GameState* gs);



	//inline Entity* CreateParticleEmitter(GameState* gs)
	//{
	//	Entity* entity = gs->CreateEntity();
	//	entity->type = EntityType::PARTICLE_EMITTER;
	//	entity->object_space_bounding_box = CreateAABBFromCenterRadius(0, 0.1f);

	//	int32 slot = -1;
	//	for (int32 i = 0; i < ArrayCount(gs->particleSlot); i++)
	//	{
	//		if (gs->particleSlot[i] == 0)
	//		{
	//			slot = i;
	//			break;
	//		}
	//	}

	//	Assert(slot != -1, "Ran out of particle emitter slots");

	//	if (slot == -1)
	//		return nullptr;

	//	entity->particlePart.emitterSlot = slot;
	//	entity->particlePart.maxLife = 1;
	//	entity->particlePart.particleCount = 0;
	//	entity->particlePart.particles = &gs->partices[slot];

	//	return entity;
	//}

	//inline Entity* CreateFireParticleEmitter(GameState* gs)
	//{
	//	Entity* entity = CreateParticleEmitter(gs);
	//	ParticleEmitterPart* emitter = &entity->particlePart;

	//	emitter->startingVelocity = Vec3f(0, 1, 0);
	//	emitter->acceleration = Vec3f(0, 0, 0);
	//	emitter->startingColour = Vec3f(0.92f, 0.13f, 0.0f) * 10.f;
	//	emitter->endingColour = Vec3f(0.97f, 0.04f, 0.04f) * 10.f;
	//	emitter->spawnRateSeconds = 0.05f;

	//	emitter->startingSize = 0.15f;
	//	emitter->endingSize = 0.05f;

	//	return entity;
	//}

	//inline Particle* CreateParticle(Entity* entity)
	//{
	//	Assert(entity->type == EntityType::PARTICLE_EMITTER, "Entity is not particle emitter");

	//	ParticleEmitterPart* emitter = &entity->particlePart;
	//	Particle* particle = nullptr;
	//	if (emitter->particleCount < MAX_PARTICLES_PER_EMITTER)
	//	{
	//		emitter->particleCount++;
	//		int32 particleIndex = 0;
	//		for (; particleIndex < MAX_PARTICLES_PER_EMITTER; particleIndex++)
	//		{
	//			particle = &emitter->particles[particleIndex];
	//			if (!particle->IsAlive())
	//			{
	//				break;
	//			}
	//		}

	//		particle->transform = Transform();
	//		particle->transform.position = entity->transform.position + RandomPointOnUnitHemisphere() * 0.5f;
	//		particle->currentLife = 3;
	//		particle->emiterSlot = emitter->emitterSlot;
	//		particle->index = particleIndex;

	//		particle->colour = emitter->startingColour;
	//		particle->velocity = emitter->startingVelocity;
	//		particle->transform.scale = emitter->startingSize;
	//		particle->transform.orientation = EulerToQuat(RandomPointOnUnitSphere() * RandomBillateral<real32>() * 30.f);
	//		particle->angularAccelertation = RandomPointOnUnitSphere() * RandomBillateral<real32>();
	//		emitter->timeSeconds = 0;
	//	}
	//	else
	//	{
	//		LOG("To many particles!!");
	//	}

	//	return particle;
	//}

	//inline void RemoveParticle(Particle* particle)
	//{
	//	particle->currentLife = 0;
	//}



	void DEBUGStepPhysics(GameState* gs, TransientState* ts, Input* input);

	////////////////////////////////////////////////////
	// @NOTE: Platform layer call this
	////////////////////////////////////////////////////
	void InitializeGameState(GameState* gs, AssetState* as, PlatformState* ws);

	void UpdateGame(GameState* gs, AssetState* as, PlatformState* ws, Input* input);

	void ConstructRenderGroup(GameState* gs, EntityRenderGroup* renderGroup);
}