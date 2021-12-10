#pragma once
#include "core/SolarCore.h"


namespace cm
{
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

		EntityId GetId();
		bool32 IsValid() const;
		bool32 IsSameEntity(Entity* other);
		AABB GetWorldBoundingBox();

		Entity* GetParent();
		Entity* GetFirstChild();
		Entity* GetSibling();
		ManagedArray<Entity*> GetChildren();
		void SetParent(EntityId entity);

		void SetLocalTransform(const Transform& transform);
		Transform GetLocalTransform();
		Transform GetWorldTransform() const;


		void EnableRendering();
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
		void SetCollider(const Sphere& sphere);
		void SetCollider(const AABB& aabb);
		void SetCollider(const AssetId& mesh, bool32 setActive = true);

		ColliderComponent GetColliderLocal() const;
		AABB GetAlignedBoxColliderLocal()const;

		ColliderComponent GetWorldCollider() const;
		Sphere GetSphereColliderWorld() const;
		AABB GetAlignedBoxColliderWorld()const;

		operator bool() const;

	private:
		EntityId id;
		EntityId parent;
		EntityId child;
		EntityId sibling;
		class Room* room;

	private:
		friend struct GameState;
		friend struct EntityId;
		friend class Room;
	};
}


