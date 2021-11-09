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

	AABB Entity::GetBoundingBox()
	{
		AABB result = UpdateAABB(this->object_space_bounding_box, this->transform.position,
			this->transform.orientation, this->transform.scale);

		return result;
	}

	void Entity::SetParent(EntityId entity)
	{
		GameState* gs = GameState::Get();
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
			if (Entity* child = entity.Get())
			{
				while (child)
				{
					Entity* next = child->sibling.Get();
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
}
