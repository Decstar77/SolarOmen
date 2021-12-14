#pragma once
#include "core/SolarCore.h"


namespace cm
{
	class Room;

	class BrainType
	{
	public:
		enum class Value
		{
			INVALID,
			PLAYER_BRAIN,
			BULLET,
			PEER_BRAIN,
			TANK_AI_IMMOBILE,
			COUNT,
		};

		BrainType()
		{
			value = Value::INVALID;
		}

		BrainType(Value v)
		{
			this->value = v;
		}

		inline bool32 IsValid() const
		{
			return this->value != Value::INVALID && this->value != Value::COUNT;
		}

		inline CString ToString() const
		{
			CString copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static BrainType ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (BrainType::Value)v;
		}

		inline static BrainType ValueOf(const CString& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INVALID;
		}

		inline bool operator==(const BrainType& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const BrainType& rhs) const
		{
			return this->value != rhs.value;
		}

		inline operator uint32() const
		{
			return (uint32)value;
		}

	private:
		Value value;

		inline static const CString __STRINGS__[] = {
			"INVALID",
			"PLAYER_BRAIN",
			"BULLET",
			"PEER_BRAIN",
			"TANK_AI_IMMOBILE",
			"COUNT"
		};
	};

	// @TODO: Refactor this !!
	enum class ColliderType
	{
		SPHERE,
		ALIGNED_BOUNDING_BOX,
		BOUDNING_BOX,
	};

	struct ColliderComponent
	{
		bool32 enabled;
		ColliderType type;
		union
		{
			Sphere sphere;
			AABB alignedBox;
		};
	};

	struct EntityId
	{
		int32 index;
		int32 generation;

		class Entity* Get() const;
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

	class Entity
	{
	public:
		CString GetName();
		void SetName(const CString& name);

		EntityId GetId() const;
		bool32 IsValid() const;
		bool32 IsSameEntity(Entity* other);
		AABB GetWorldBoundingBox();

		Entity* GetParent();
		Entity* GetFirstChild();
		Entity* GetSiblingAhead();
		Entity* GetSiblingBehind();
		ManagedArray<Entity*> GetChildren();
		void SetParent(Entity entity);

		void SetLocalTransform(const Transform& transform);
		Transform GetLocalTransform() const;
		Transform GetWorldTransform() const;

		void EnableRendering();
		void SetRendering(const CString& modelName, const CString& textureName);

		void SetModel(const AssetId& id);
		void SetModel(const CString& name);
		AssetId GetModel() const;

		void SetTexture(const AssetId& id);
		void SetTexture(const CString& name);
		AssetId GetTexture() const;

		void SetShader(const AssetId& id);
		void SetShader(const CString& name);
		AssetId GetShader() const;

		// @NOTE: These are all in local space
		void EnableCollider();
		void SetCollider(const Sphere& sphere, bool32 enabled = true);
		void SetCollider(const AABB& aabb, bool32 enabled = true);
		void SetCollider(const AssetId& mesh, bool32 setActive = true);

		ColliderComponent GetColliderLocal() const;
		AABB GetAlignedBoxColliderLocal()const;

		ColliderComponent GetWorldCollider() const;
		Sphere GetSphereColliderWorld() const;
		AABB GetAlignedBoxColliderWorld()const;

		struct BrainComponent* SetBrain(BrainType brainType, bool32 enable = true);
		struct BrainComponent* GetBrain();

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




	enum class SnapShotType : uint8
	{
		INVALID = 0,
		HANDSHAKE_CONNECTION,
		PING,
		TRANSFORM,
		BULLET_SHOT,
		TICK,
	};

	struct SnapShotTransform
	{
		SnapShotType type;
		Vec3f tankPosition;
		Quatf tankOrientation;

		Vec3f turretPosition;
		Quatf turretOrientation;
	};

	struct SnapShotBulletShot
	{
		uint32 bulletId;
		bool ack;
		Vec3f position;
		Quatf orientation;
	};

	struct SnapGameTick
	{
		int32 tickNumber;
		bool playerSpawnBullet;
		Vec3f tankPosition;
		Quatf tankOrientation;
		Vec3f turretPosition;
		Quatf turretOrientation;
	};

	struct SnapShotPing
	{
		bool ack;
	};

	struct SnapShot
	{
		SnapShotType type;
		union
		{
			SnapShotTransform snapTransform;
			SnapShotBulletShot snapBullet;
			SnapGameTick snapTick;
			SnapShotPing snapPing;
		};
	};

	static_assert(sizeof(SnapShotTransform) < 256);

	struct PlayerBrain
	{
		static constexpr real32 TANK_MOVE_SPEED = 2.0f;
		static constexpr real32 TANK_ROTATION_SPEED = 2.0f;

		static constexpr real32 FIRE_RATE = 0.1f;
		real32 lastFireTime = 0.0f;
		bool canFire = false;

		Entity tank;
		Entity turret;
		Entity bulletSpawnPoint;

		bool32 initialized;

		Vec2f tankVelocity;
		real32 turretRotation;
		real32 tankRotation;

		void FrameUpdate(Room* room, real32 dt);
		void TickUpdate(Room* room, real32 dt);

	private:
		void Start(Room* room, real32 dt);
		void UpdateTurret(Room* room, real32 dt);
		void UpdateBase(Room* room, real32 dt);
		void UpdateFiring(Room* room, real32 dt);
	};

	struct BulletBrain
	{
		static constexpr real32 BULLET_MOVE_SPEED = 4.0f;
		bool32 initialized;
		int32 collisionCount;

		void FrameUpdate(Room* room, Entity entity, real32 dt);
	};

	struct PeerBrain
	{
		static constexpr int32 PACKETS_PER_SECOND = 30;

		real32 timeTillLastSend;

		Entity player1Tank;
		Entity player1Turret;

		Entity player2Tank;
		Entity player2Turret;


		void FrameUpdate(Room* room, Entity entity, real32 dt);
	};

	struct TankAIImmobile
	{
		static constexpr real32 SCANNING_TURN_RATE = DegToRad(15.0f);
		static constexpr real32 SCANNING_LOCKED_RATE = DegToRad(145.0f);
		static constexpr real32 FIRE_RATE = 1.0f; // @NOTE: shots per second

		Entity player1Tank;
		Entity player2Tank;

		Entity tank;
		Entity bulletSpawnPoint;

		real32 rotation = 0.0f;
		real32 lastFireTime = 0.0f;

		void FrameUpdate(Room* room, Entity entity, real32 dt);
	};

	struct BrainComponent
	{
		bool32 enabled;
		BrainType type;
		union
		{
			PlayerBrain playerBrain;
			BulletBrain bulletBrain;
			PeerBrain networkBrain;
			TankAIImmobile tankAIImmobile;
		};
	};


}


