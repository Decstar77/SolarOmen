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

	void Entity::SetCollider(const Sphere& sphere)
	{
		Assert(IsValid(), "Entity invalid");
		ColliderComponent* cc = &room->colliderComponents[id.index];
		cc->type = ColliderType::SPHERE;
		cc->sphere = sphere;
	}

	void Entity::SetCollider(const AABB& aabb)
	{
		Assert(IsValid(), "Entity invalid");
		ColliderComponent* cc = &room->colliderComponents[id.index];
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

	Entity::operator bool() const
	{
		return IsValid();
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

	Transform Entity::GetWorldTransform() const
	{
		Assert(IsValid(), "Entity invalid");
		return room->transformComponents[id.index].transform;
	}

	void Entity::SetLocalTransform(const Transform& transform)
	{
		Assert(IsValid(), "Entity invalid");
		room->transformComponents[id.index].transform = transform;
	}

	void Entity::EnableRendering()
	{
		Assert(IsValid(), "Entity invalid");
		room->renderComponents[id.index].enabled = true;
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