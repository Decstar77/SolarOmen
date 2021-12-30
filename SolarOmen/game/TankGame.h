#pragma once
#include "components/SolarCamera.h"
#include "Entity.h"
#include "Multiplayer.h"
#include "ManifoldTests.h"
namespace cm
{
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

	class GridCellType
	{
	public:
		enum class Value
		{
			EMPTY = 0,
			FLOOR,
			WALL,
			COUNT,
		};

		GridCellType()
		{
			value = Value::EMPTY;
		}

		GridCellType(Value v)
		{
			this->value = v;
		}

		inline CString ToString() const
		{
			CString copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static GridCellType ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (GridCellType::Value)v;
		}

		inline static GridCellType ValueOf(const CString& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::EMPTY;
		}

		inline bool operator==(const GridCellType& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const GridCellType& rhs) const
		{
			return this->value != rhs.value;
		}

		inline operator uint32() const
		{
			return (uint32)value;
		}

		inline static const CString __STRINGS__[] = {
			"EMPTY",
			"FLOOR",
			"WALL",
			"COUNT"
		};

	private:
		Value value;
	};

	struct GridCell
	{
		int32 xIndex;
		int32 yIndex;
		int32 index;
		Entity entity;
		Vec2f position;
		GridCellType type;
	};

	inline int32 IndexOf2DArray(int32 width, int32 x, int32 y) { return y * width + x; }

	struct Grid
	{
		static constexpr uint32 HORIZONTAL_CELL_COUNT = RoomAsset::ROOM_HORIZTONAL_SIZE;
		static constexpr uint32 VERTICAL_CELL_COUNT = RoomAsset::ROOM_VERTICAL_SIZE;
		static constexpr real32 CELL_EXTENT = 2.0f;

		Vec2f topLeft;
		Vec2f topRight;
		Vec2f bottomRight;
		Vec2f bottomLeft;
		Vec2f center;

		FixedArray<GridCell, HORIZONTAL_CELL_COUNT* VERTICAL_CELL_COUNT> cells;

		GridCell* GetCellFromPosition(const Vec3f& position);
		bool IsValidIndex(int32 xIndex, int32 yIndex);
		void Initialize(const FixedArray<int32, HORIZONTAL_CELL_COUNT* VERTICAL_CELL_COUNT>* map);
		void DebugDraw();
	};

	struct RaycastResult
	{
		bool32 hit;
		Entity entity;
		RaycastInfo rayInfo;
	};

	struct UIText
	{
		CString text;
		real32 oX;
		real32 oY;
		real32 scale;
	};

	struct UIRect
	{
		real32 oX;
		real32 oY;
		real32 width;
		real32 height;
		AssetId texture;
		Vec4f colour;
	};

	enum class UIElementType
	{
		INVALID = 0,
		TEXT,
		RECT,
		BUTTON,
	};

	struct UIElement
	{
		UIElementType type;
		union
		{
			UIText text;
			UIRect rect;
		};
	};

	struct UserInterfaceState
	{
		real32 totalTime;
		uint32 selectionPos;
		real32 selectionScale;

		FixedArray<UIElement, 256> uiElements;

		void Start();
		void Window(uint32 id);
		void Rect(real32 oX, real32 oY, real32 halfWidth, real32 halfHeight, const Vec4f& colour);
		void Rect(real32 oX, real32 oY, real32 halfWidth, real32 halfHeight, AssetId textureId, const Vec4f& colour = Vec4f(1, 1, 1, 1));
		void Text(const CString& text, real32 oX, real32 oY, real32 scale);
		bool Button(const CString& text, real32 oX, real32 oY, real32 halfWidth, real32 halfHeight, real32 txtScale = 1.0f);
		void End();
	};

	class Room
	{
	public:
		static constexpr uint32 ENTITY_STORAGE_COUNT = 1000;
		static constexpr uint32 INVALID_ENTITY_INDEX = 0;

		AssetId id;
		CString name;
		RoomType type;

		bool twoPlayerGame;
		bool initialized;
		bool isPaused;

		real32 totalTime;

		Camera playerCamera;
		Vec3f playerCameraOffset;

		Grid grid;
		Entity gridEntity;

		UserInterfaceState uiState;
		FixedArray<GameCommand, 256> commands;
		MultiplayerState* multiplayerState;

		FixedArray<Entity, 256> friendlyTanks;
		FixedArray<Entity, 256> enemyTanks;
		FixedArray<Entity, 1024> bullets;

		FixedArray<Entity, ENTITY_STORAGE_COUNT> entities;
		FixedArray<EntityId, ENTITY_STORAGE_COUNT - 1> entityFreeList;

		FixedArray<TransformComponent, ENTITY_STORAGE_COUNT> transformComponents;
		FixedArray<NameComponent, ENTITY_STORAGE_COUNT> nameComponents;
		FixedArray<RenderComponent, ENTITY_STORAGE_COUNT> renderComponents;
		FixedArray<ColliderComponent, ENTITY_STORAGE_COUNT> colliderComponents;
		FixedArray<BrainComponent, ENTITY_STORAGE_COUNT> brainComponents;
		FixedArray<NetworkComponent, ENTITY_STORAGE_COUNT> networkComponents;
		FixedArray<TagComponent, ENTITY_STORAGE_COUNT> tagComponents;

		uint32 entityLoopIndex = 0;
	public:
		void BeginEntityLoop();
		Entity GetNextEntity();

		Entity CreateEntity();
		Entity CreateEntity(const CString& name);

		void DestoryEntity(Entity* entity);

		Entity SpawnBullet(Transform transform);
		Entity SpawnEnemyTank(const Vec3f& pos);
		RaycastResult ShootRayThrough(const Ray& ray);

		void GameCommandSpawnBullet(const Vec3f& pos, const Quatf& ori);
		void GameCommandDestroyEntity(Entity entity);

		void CreateEntitiesFromGripMap();

		void Initialize(const RoomAsset& roomAsset, MultiplayerState* mulitplayerState);
		void Update(real32 dt);
		void Shutdown();
		void ConstructRenderGroup(EntityRenderGroup* renderGroup);

		void DEBUGEnableTickCalls();
		void DEBUGDisableTickCalls();
		void DEBUGDrawAllColliders();

	private:
		void PerformGameCommands(FixedArray<GameCommand, 256>* commands, PlayerNumber playerNumber);

		void CreatePeerTank(const Vec3f& position);
		void CreateHostTank(const Vec3f& position);

		void InitializeMenuRoom();
		void UpdateMenuRoom(real32 dt);
		void ShutdownMenuRoom();

		void InitializeLevelSelectionRoom();
		void UpdateLevelSelectionRoom(real32 dt);
		void ShutdownLevelSelectionRoom();

		void InitializeMultiplayerRoom();
		void UpdateMultiplayerRoom(real32 dt);
		void ShutdownMultiplayeRoom();

		void InitializeGameRoom(const RoomAsset& roomAsset);
		void UpdateGameRoom(real32 dt);
		void ShutdownGameRoom();

		void CreateEntityFreeList();
		EntityId GetNextFreeEntityId();
		void PushFreeEntityId(EntityId id);
		void RemoveEntityChildParentRelationship(Entity* entity);

	};

#define GetGameState() GameState *gs = GameState::Get()
	struct GameState
	{
		MultiplayerState multiplayerState;

		RoomType nextRoom;
		Room currentRoom;

		void TransitionToRoom(const RoomAsset& roomAsset);

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

		UserInterfaceState uiState;
		FixedArray<RenderEntry, 1000> entries;
	};
}