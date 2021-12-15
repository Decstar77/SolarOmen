#include "Entity.h"
#include "TankGame.h"

namespace cm
{
	Entity* EntityId::Get() const
	{
		GetGameState();
		Room* room = &gs->currentRoom;

		if (index > 0 && index < (int32)room->entities.GetCapcity())
		{
			Entity* stored = &room->entities[index];
			if (stored->id == *this)
			{
				return stored;
			}
		}

		return nullptr;
	}


	EntityId Entity::GetId() const
	{
		return id;
	}

	bool32 Entity::IsValid() const
	{
		return id.Get() != nullptr;
	}

	void Entity::EnableCollider()
	{
		Assert(IsValid(), "Entity invalid");
		ColliderComponent* cc = &room->colliderComponents[id.index];
		cc->enabled = true;
	}

	void Entity::SetCollider(const Sphere& sphere, bool32 enabled)
	{
		Assert(IsValid(), "Entity invalid");
		ColliderComponent* cc = &room->colliderComponents[id.index];
		cc->enabled = enabled;
		cc->type = ColliderType::SPHERE;
		cc->sphere = sphere;
	}

	void Entity::SetCollider(const AABB& aabb, bool32 enabled)
	{
		Assert(IsValid(), "Entity invalid");
		ColliderComponent* cc = &room->colliderComponents[id.index];
		cc->enabled = enabled;
		cc->type = ColliderType::ALIGNED_BOUNDING_BOX;
		cc->alignedBox = aabb;
	}

	ColliderComponent Entity::GetColliderLocal() const
	{
		Assert(IsValid(), "Entity invalid");
		ColliderComponent cc = room->colliderComponents[id.index];

		return cc;
	}

	AABB Entity::GetAlignedBoxColliderLocal() const
	{
		ColliderComponent cc = GetColliderLocal();
		Assert(cc.type == ColliderType::ALIGNED_BOUNDING_BOX, "Collider is not of type aabb");

		return cc.alignedBox;
	}

	Sphere Entity::GetSphereColliderLocal() const
	{
		ColliderComponent cc = GetColliderLocal();
		Assert(cc.type == ColliderType::SPHERE, "Collider is not of type sphere");

		return cc.sphere;
	}

	Sphere Entity::GetSphereColliderWorld() const
	{
		ColliderComponent cc = GetColliderLocal();
		Transform tr = GetWorldTransform();

		Assert(cc.type == ColliderType::SPHERE, "Collider is not of type sphere");
		Sphere sphere = TranslateSphere(cc.sphere, tr.position);

		return sphere;
	}

	AABB Entity::GetAlignedBoxColliderWorld() const
	{
		ColliderComponent cc = GetColliderLocal();
		Transform tr = GetWorldTransform();
		Assert(cc.type == ColliderType::ALIGNED_BOUNDING_BOX, "Collider is not of type aabb");

		AABB box = UpdateAABB(cc.alignedBox, tr.position, tr.orientation, tr.scale);

		return box;
	}

	BrainComponent* Entity::SetBrain(BrainType brainType, bool32 enable)
	{
		Assert(IsValid(), "Entity invalid");
		BrainComponent* bc = &room->brainComponents[id.index];
		ZeroStruct(bc);
		bc->enabled = enable;
		bc->type = brainType;
		return bc;
	}

	BrainComponent* Entity::GetBrain()
	{
		Assert(IsValid(), "Entity invalid");
		BrainComponent* bc = &room->brainComponents[id.index];
		return bc;
	}

	Entity::operator bool() const
	{
		return IsValid();
	}

	bool Entity::operator==(const Entity& rhs) const
	{
		return this->id == rhs.id;
	}

	bool Entity::operator!=(const Entity& rhs) const
	{
		return this->id != rhs.id;
	}

	CString Entity::GetName()
	{
		Assert(IsValid(), "Entity invalid");
		return room->nameComponents[id.index].name;
	}

	void Entity::SetName(const CString& name)
	{
		Assert(IsValid(), "Entity invalid");
		room->nameComponents[id.index].name = name;
	}

	Entity* Entity::GetParent()
	{
		return parent.Get();
	}

	Entity* Entity::GetFirstChild()
	{
		return child.Get();
	}

	Entity* Entity::GetSiblingAhead()
	{
		return siblingAhead.Get();
	}

	Entity* Entity::GetSiblingBehind()
	{
		return siblingBehind.Get();
	}

	void Entity::SetParent(Entity entity)
	{
		Assert(IsValid(), "Entity invalid");
		Assert(this->id != entity.id, "Cannot parent an entity to its self !!");

		if (Entity* existingParent = parent.Get())
		{
			Entity* prev = siblingBehind.Get();
			Entity* next = siblingAhead.Get();

			// @NOTE: I(this) was the only child
			if (!prev && !next)
			{
				existingParent->child = {};
			}
			// @NOTE: me was the last child
			else if (prev && !next)
			{
				prev->siblingAhead = {};
			}
			// @NOTE: me the the first child
			else if (!prev && next)
			{
				existingParent->child = next->id;
				next->siblingBehind = {};
			}
			// @NOTE: me was a middle child
			else if (prev && next)
			{
				prev->siblingAhead = next->id;
				next->siblingBehind = prev->id;
			}
			else
			{
				Assert(0, "SetParent went wrong");
			}
		}

		Entity* stored = id.Get();
		if (Entity* newParent = entity.id.Get())
		{


			stored->parent = newParent->id;

			if (Entity* child = newParent->GetFirstChild())
			{
				while (child)
				{
					Entity* next = child->GetSiblingAhead();
					if (next != nullptr)
					{
						child = next;
					}
					else
					{
						break;
					}
				}

				child->siblingAhead = stored->id;
				stored->siblingBehind = child->id;
			}
			else
			{
				newParent->child = stored->id;
			}
		}
		else
		{
			stored->parent = {};
			stored->siblingBehind = {};
			stored->siblingAhead = {};
		}

		*this = *stored;
	}

	void Entity::SetLocalTransform(const Transform& transform)
	{
		Assert(IsValid(), "Entity invalid");
		room->transformComponents[id.index].transform = transform;
	}

	Transform Entity::GetLocalTransform() const
	{
		Assert(IsValid(), "Entity invalid");
		return room->transformComponents[id.index].transform;
	}

	Transform Entity::GetWorldTransform() const
	{
		Assert(IsValid(), "Entity invalid");

		Transform worldTransfom = room->transformComponents[id.index].transform;
		Entity* parentEntity = parent.Get();
		if (parentEntity)
		{
			worldTransfom = Transform::CombineTransform(worldTransfom, parentEntity->GetWorldTransform());
		}

		return worldTransfom;
	}

	void Entity::EnableRendering()
	{
		Assert(IsValid(), "Entity invalid");
		room->renderComponents[id.index].enabled = true;
	}

	void Entity::SetRendering(const CString& modelName, const CString& textureName)
	{
		EnableRendering();
		SetModel(modelName);
		SetTexture(textureName);
	}

	void Entity::SetModel(const CString& name)
	{
		Assert(IsValid(), "Entity invalid");
		GetAssetState();
		ManagedArray<ModelAsset> models = as->models.GetValueSet();
		RenderComponent* render = &room->renderComponents[id.index];
		render->modelId = GetAssetFromName<ModelAsset>(models, name).id;
	}

	void Entity::SetTexture(const CString& name)
	{
		Assert(IsValid(), "Entity invalid");
		GetAssetState();

		ManagedArray<TextureAsset> textures = as->textures.GetValueSet();
		RenderComponent* render = &room->renderComponents[id.index];
		render->textureId = GetAssetFromName<TextureAsset>(textures, name).id;
	}
}