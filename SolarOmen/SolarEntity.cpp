#include "SolarOmen.h"

namespace cm
{
	Entity* EntityId::Get()
	{
		GameState* gs = GameState::Get();

		if (index > 0 && index <= ArrayCount(gs->entites))
		{
			Entity* stored = &gs->entites[index];
			if (stored->id == *this)
			{
				return stored;
			}
		}

		return nullptr;
	}

	CString EntityId::ToString()
	{
		return CString("Index=").Add(index).Add(" Generation=").Add(generation);
	}

	CString* Entity::GetName()
	{
		if (IsValid())
		{
			return &gs->nameComponents[id.index];
		}

		return nullptr;
	}

	void Entity::SetName(const CString& name)
	{
		if (IsValid())
		{
			gs->nameComponents[id.index] = name;
		}
	}

	EntityId Entity::GetId()
	{
		return id;
	}

	bool32 Entity::IsValid()
	{
		return id.Get() != nullptr;
	}

	bool32 Entity::IsSameEntity(Entity* other)
	{
		bool32 result = this->id == other->id;

		return result;
	}

	bool32 Entity::IsPhsyicsEnabled()
	{
		int32 flags = (int32)this->flags;
		return flags & (int32)EntityFlag::SIMULATE_PHYSICS;
	}

	AABB Entity::GetWorldBoundingBox()
	{
		Transform worldTransform = GetWorldTransform();

		AABB result = UpdateAABB(this->boundingBoxLocal, worldTransform.position,
			worldTransform.orientation, worldTransform.scale);

		return result;
	}

	Entity* Entity::GetParent()
	{
		return parent.Get();
	}

	Entity* Entity::GetFirstChild()
	{
		return child.Get();
	}

	Entity* Entity::GetSibling()
	{
		return sibling.Get();
	}

	ManagedArray<Entity*> Entity::GetChildren()
	{
		// @TODO: When we do dirty and stuff we could probably cache the count somewhere
		ManagedArray<Entity*> children = GameMemory::PushTransientArray<Entity*>(128);

		Entity* child = GetFirstChild();
		while (child)
		{
			int32 index = children.count;
			children[index] = child;
			children.count++;

			child = child->GetSibling();
		}

		return children;
	}

	void Entity::SetParent(EntityId entity)
	{
		Assert(IsValid(), "SetParent me is an invalid entities");

		//@NOTE: Do we have an existing parent
		if (Entity* existingParent = parent.Get())
		{
			// @NOTE: prev -> me -> next
			Entity* prev = nullptr;
			Entity* next = nullptr;

			// @NOTE: Find prev and next
			Entity* child = existingParent->child.Get();
			while (child)
			{
				if (child->sibling == this->id)
				{
					prev = child;
				}
				if (this->sibling == child->id)
				{
					next = child;
				}

				child = child->sibling.Get();
			}

			// @NOTE: I(this) was the only child
			if (!prev && !next)
			{
				existingParent->child = {};
			}
			// @NOTE: me was the last child
			else if (prev && !next)
			{
				prev->sibling = {};
			}
			// @NOTE: me the the first child
			else if (!prev && next)
			{
				existingParent->child = next->id;
			}
			// @NOTE: me was a middle child
			else if (prev && next)
			{
				prev->sibling = next->id;
			}
			else
			{
				Assert(0, "SetParent went wrong");
			}
		}

		if (Entity* parent = entity.Get())
		{
			this->parent = parent->id;

			// @NOTE: The new parent has child, so place me(this) at the back
			if (Entity* child = parent->GetFirstChild())
			{
				while (child)
				{
					Entity* next = child->GetSibling();
					if (next != nullptr)
					{
						child = next;
					}
					else
					{
						break;
					}
				}

				child->sibling = this->id;
			}
			// @NOTE: New parent has no child
			else
			{
				parent->child = this->id;
			}
		}
		else
		{
			this->parent = {};
			this->sibling = {};
		}
	}

	Transform Entity::GetLocalTransform()
	{
		return transform;
	}

	Transform Entity::GetWorldTransform()
	{
		Mat4f me = transform.CalculateTransformMatrix();

		Entity* next = parent.Get();
		if (next)
		{
			me = me * next->GetWorldTransform().CalculateTransformMatrix();
		}

		return Transform(me);

		//while (next && next->IsValid())
		//{
		//	Mat4f other = next->GetLocalTransform().CalculateTransformMatrix();
		//
		//	me = me * other;
		//
		//	next = next->parent.Get();
		//}
		//
		//return me;
	}

	void Entity::SetCollider(const ModelId& mesh, bool32 setActive)
	{
		Assert(mesh.IsValid(), "Invalid mesh collider");

		collisionComp.active = setActive;
		collisionComp.type = ColliderType::MESH;
		collisionComp.meshIndex = (int32)mesh;

		boundingBoxLocal = collisionComp.GetBoundingBox();
	}

	RacingWaypoint* Entity::GetRacingWaypointComponent()
	{
		Assert(IsValid(), "GetRacingWaypointComponent, entity is not valid");

		RacingWaypoint* comp = &gs->racingWaypointComponents[id.index];

		return comp;
	}


	AABB CollisionComponent::GetBoundingBox()
	{
		AABB box = {};
		switch (type)
		{
		case ColliderType::SPHERE: Assert(0, ""); break;
		case ColliderType::BOX: Assert(0, ""); break;
		case ColliderType::MESH:
		{
			GameState* gs = GameState::Get();
			Assert(meshIndex >= 0 && meshIndex < gs->meshColliderCount, "Collision mesh index is invalid");
			if (meshIndex >= 0 && meshIndex < gs->meshColliderCount)
			{
				box = gs->meshColliders[meshIndex].boundingBox;
			}
			break;
		}
		}

		return box;
	}
}
