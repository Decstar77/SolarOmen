#pragma once
#include "core/SolarCore.h"
#include "EntityId.h"

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

	struct TransformComponent
	{
		Transform transform;
	};

	class Tag
	{
	public:
		enum class Value
		{
			NONE = 0,
			BULLET,
			TEAM1_TANK,
			TEAM2_TANK,
			COUNT,
		};

		Tag()
		{
			value = Value::NONE;
		}

		Tag(Value v)
		{
			this->value = v;
		}

		inline CString ToString() const
		{
			CString copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline Value Get() const { return value; }

		inline static Tag ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (Tag::Value)v;
		}

		inline static Tag ValueOf(const CString& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::NONE;
		}

		inline bool operator==(const Tag& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const Tag& rhs) const
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
			"NONE",
			"BULLET",
			"TEAM1_TANK",
			"TEAM2_TANK",
			"COUNT"
		};
	};

	struct TagComponent
	{
		Tag tag;
	};

	struct NetworkComponent
	{
		PlayerNumber playerOwner;
		bool32 markedDestroyed;
		Vec3f lerpPosition;
		Quatf lerpOrientation;
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

		void SetTag(const Tag& tag);
		Tag GetTag() const;

		void SetNetworkOwner(const PlayerNumber& owner);

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
		AABB GetAlignedBoxColliderLocal() const;
		Sphere GetSphereColliderLocal() const;

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

	struct PlayerBrain
	{
		static constexpr real32 TANK_MOVE_SPEED = 2.0f;
		static constexpr real32 TANK_ROTATION_SPEED = 2.0f;

		static constexpr real32 FIRE_RATE = 1.1f;
		real32 lastFireTime = 0.0f;
		bool canFire = false;

		Entity visualTank;
		Entity visualTurret;
		real32 visualTurretRotation;
		real32 visualTankRotation;

		void FrameUpdate(Room* room, Entity entity, real32 dt);

	private:
		void UpdateTurret(Room* room, real32 dt);
		void UpdateBase(Room* room, real32 dt);
		void UpdateFiring(Room* room, real32 dt);
	};

	inline static constexpr real32 NETWORK_INTERPOLATE_AMOUNT = 0.65f;

	struct BulletBrain
	{
		static constexpr real32 BULLET_MOVE_SPEED = 8.0f;
		bool32 initialized;
		int32 collisionCount;
		int32 index;

		Vec2f moveDir;
		Vec2f moveDelta;

		void FrameUpdate(Room* room, Entity entity, real32 dt);
	};

	struct TankAIImmobile
	{
		static constexpr real32 SCANNING_TURN_RATE = DegToRad(15.0f);
		static constexpr real32 SCANNING_LOCKED_RATE = DegToRad(145.0f);
		static constexpr real32 FIRE_RATE = 1.0f; // @NOTE: shots per second

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
			TankAIImmobile tankAIImmobile;
		};
	};


}


