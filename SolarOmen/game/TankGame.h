#pragma once
#include "components/SolarCamera.h"
#include "Entity.h"
#include "ManifoldTests.h"
namespace cm
{
	struct TransformComponent
	{
		bool32 cached;
		Transform transform;
		Transform worldTransform;
	};

	struct NameComponent
	{
		CString name;
	};

	struct RenderComponent
	{
		bool32 enabled;
		AssetId modelId;
		AssetId textureId;
		AssetId shaderId;
	};

	struct GridCell
	{
		int32 xIndex;
		int32 yIndex;
		int32 index;
		bool32 occupied;
		Entity entity;
		Vec2f position;
	};

	inline int32 IndexOf2DArray(int32 width, int32 x, int32 y) { return y * width + x; }

	struct Grid
	{
		static constexpr uint32 HORIZONTAL_CELL_COUNT = 10;
		static constexpr uint32 VERTICAL_CELL_COUNT = 10;
		static constexpr real32 CELL_EXTENT = 2.0f;

		Vec2f topLeft;
		Vec2f topRight;
		Vec2f bottomRight;
		Vec2f bottomLeft;
		Vec2f center;

		FixedArray<GridCell, HORIZONTAL_CELL_COUNT* VERTICAL_CELL_COUNT> cells;

		void Initialize();
		void DebugDraw();
	};

	struct RaycastResult
	{
		bool32 hit;
		Entity entity;
		RaycastInfo rayInfo;
	};

	class Room
	{
	public:
		static constexpr uint32 ENTITY_STORAGE_COUNT = 1000;
		static constexpr uint32 INVALID_ENTITY_INDEX = 0;

		Camera playerCamera;
		Vec3f playerCameraOffset;

		Grid grid;

		SnapShot snapShotStorage[256];
		Queue<SnapShot> snapShots;

		FixedArray<Entity, ENTITY_STORAGE_COUNT> bullets;

		FixedArray<Entity, ENTITY_STORAGE_COUNT> entities;
		FixedArray<EntityId, ENTITY_STORAGE_COUNT - 1> entityFreeList;

		FixedArray<TransformComponent, ENTITY_STORAGE_COUNT> transformComponents;
		FixedArray<NameComponent, ENTITY_STORAGE_COUNT> nameComponents;
		FixedArray<RenderComponent, ENTITY_STORAGE_COUNT> renderComponents;
		FixedArray<ColliderComponent, ENTITY_STORAGE_COUNT> colliderComponents;
		FixedArray<BrainComponent, ENTITY_STORAGE_COUNT> brainComponents;

		uint32 entityLoopIndex = 0;
	public:
		Entity CreateEntity();
		Entity CreateEntity(const CString& name);

		// @TODO: Remember to reconstruct parent/child and clear any components too !!
		void DestoryEntity(Entity entity);

		void BeginEntityLoop();
		Entity GetNextEntity();

		RaycastResult ShootRayThrough(const Ray& ray);

		void Initialize();
		void Update(real32 dt);
		void ConstructRenderGroup(EntityRenderGroup* renderGroup);
		void Shutdown();


		void DEBUGDrawAllColliders();

	private:
		void CreateEntityFreeList();
		EntityId GetNextFreeEntityId();
		void PushFreeEntityId(EntityId id);

	};

#define GetGameState() GameState *gs = GameState::Get()
	struct GameState
	{
		Room currentRoom;

		static inline void Initialize(GameState* gs) { gameState = gs; }
		static inline GameState* Get() { return gameState; }
	private:
		inline static GameState* gameState = nullptr;
	};

	// @TODO: I think this could just live in solar renderer
	enum class RenderFlag : uint32
	{
		NONE = 0,
		REQUIRES_ALPHA_TESTING = SetABit(1),
		NO_CAST_SHADOW = SetABit(2),
		NO_RECEIVES_SHADOW = SetABit(3)
	};

	struct RenderEntry
	{
		Transform transform; // @NOTE: In world space
		AssetId modelId;
		AssetId textureId;
		AssetId shaderId;
	};

	struct EntityRenderGroup
	{
		Camera mainCamera;
		Camera playerCamera;

		FixedArray<RenderEntry, 1000> entries;
	};
}