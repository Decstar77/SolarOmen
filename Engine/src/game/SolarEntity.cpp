#include "SolarGameTypes.h"
#include "core/SolarLogging.h"
#include "core/SolarInput.h"

namespace sol
{
	SOL_API Ray Camera::ShootRayAtMouse() const
	{
		real32 width = (real32)Application::GetSurfaceWidth();
		real32 height = (real32)Application::GetSurfaceHeight();
		real32 aspect = Application::GetSurfaceAspectRatio();

		Vec2f pixelPoint = Input::Get()->mousePositionPixelCoords;

		Mat4f proj = PerspectiveLH(DegToRad(yfov), aspect, near_, far_);
		Mat4f view = Inverse(transform.CalculateTransformMatrix());

		Vec4f normal_coords = GetNormalisedDeviceCoordinates(width,
			height, pixelPoint.x, pixelPoint.y);

		Vec4f view_coords = ToViewCoords(proj, normal_coords);

		// @NOTE: This 1 ensure we a have something pointing in to the screen
		view_coords = Vec4f(view_coords.x, view_coords.y, 1, 0);
		Vec3f world_coords = ToWorldCoords(view, view_coords);

		Ray ray = {};
		ray.origin = transform.position;
		ray.direction = Normalize(Vec3f(world_coords.x, world_coords.y, world_coords.z));

		return ray;
	}

	String Entity::GetName() const
	{
		Assert(IsValid(), "Entity invalid");
		return room->nameComponents[id.index].name;
	}

	void Entity::SetName(const String& name)
	{
		Assert(IsValid(), "Entity invalid");
		room->nameComponents[id.index].name = name;
	}

	EntityId Entity::GetId() const
	{
		return id;
	}

	bool Entity::IsValid() const
	{
		return id.Get() != nullptr;
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

	void Entity::SetParent(Entity entity)
	{
		Assert(IsValid(), "Entity invalid");
		Assert(this->id != entity.id, "Cannot parent an entity to its self !!");

		TransformComponent* me = &room->transformComponents[this->id.index];

		if (!entity.IsValid())
		{
			if (me->parent.index != 0) {
				TransformComponent* currentParnet = &room->transformComponents[me->parent.index];
				currentParnet->children.Remove(this->id);
			}

			me->parent = {};
			return;
		}

		TransformComponent* other = &room->transformComponents[entity.id.index];

		ManagedArray<Entity> descendants = GetDescendants();
		for (uint32 i = 0; i < descendants.count; i++)
		{
			if (descendants[i].id == entity.id)
			{
				SOLWARN("Attempting to set a parent to a descendant");
				return;
			}
		}

		if (me->parent.index != 0)
		{
			TransformComponent* currentParnet = &room->transformComponents[me->parent.index];
			currentParnet->children.Remove(this->id);
		}

		me->parent = entity.id;
		other->children.Add(this->id);
	}

	Entity Entity::GetParent()
	{
		Assert(IsValid(), "Entity invalid");
		TransformComponent* me = &room->transformComponents[id.index];
		Entity parent = {};
		parent.id = me->parent;

		return parent;
	}

	ManagedArray<Entity> Entity::GetChildren()
	{
		Assert(IsValid(), "Entity invalid");
		TransformComponent* me = &room->transformComponents[id.index];
		ManagedArray<Entity> children = ManagedArray<Entity>(me->children.count, MemoryType::TRANSIENT);
		for (uint32 i = 0; i < me->children.count; i++)
		{
			Entity entity = {};
			entity.id = me->children[i];
			children.Add(entity);
		}

		return children;
	}

	uint32 Entity::GetChildrenCount()
	{
		Assert(IsValid(), "Entity invalid");
		TransformComponent* me = &room->transformComponents[id.index];
		return me->children.count;
	}

	uint32 Entity::GetDescendantCount()
	{
		Assert(IsValid(), "Entity invalid");
		TransformComponent* me = &room->transformComponents[id.index];

		uint32 count = me->children.count;
		for (uint32 i = 0; i < me->children.count; i++)
		{
			count += me->children[i].Get()->GetDescendantCount();
		}

		return count;
	}

	void Entity::DoGetDescendants(Entity entity, ManagedArray<Entity>* des)
	{
		des->Add(entity);
		TransformComponent* me = &room->transformComponents[entity.id.index];
		for (uint32 i = 0; i < me->children.count; i++)
		{
			Entity child = {};
			child.id = me->children[i];
			DoGetDescendants(child, des);
		}
	}

	ManagedArray<Entity> Entity::GetDescendants()
	{
		Assert(IsValid(), "Entity invalid");
		TransformComponent* me = &room->transformComponents[id.index];
		uint32 count = GetDescendantCount();

		ManagedArray<Entity> des = ManagedArray<Entity>(count, MemoryType::TRANSIENT);
		for (uint32 i = 0; i < me->children.count; i++)
		{
			Entity child = {};
			child.id = me->children[i];
			DoGetDescendants(child, &des);
		}

		return des;
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
		return room->transformComponents[id.index].transform;
		//Transform worldTransfom = room->transformComponents[id.index].transform;
		//Entity* parentEntity = parent.Get();
		//if (parentEntity)
		//{
		//	worldTransfom = Transform::CombineTransform(worldTransfom, parentEntity->GetWorldTransform());
		//}

		return Transform();
	}

	MaterialComponent* Entity::GetMaterialomponent()
	{
		Assert(IsValid(), "Entity invalid");
		return &room->materialComponets[id.index];
	}

	void Entity::SetMaterial(const String& modelName, const String& albedo)
	{
		Assert(IsValid(), "Entity invalid");
		ModelResource* model = Resources::GetModelResource(modelName);
		TextureResource* texture = Resources::GetTextureResource(albedo);

		if (model)
		{
			room->materialComponets[id.index].material.modelId = model->id;
		}
		else
		{
			SOLERROR("Model resource not found");
		}

		if (texture)
		{
			room->materialComponets[id.index].material.albedoId = texture->id;
		}
		else
		{
			SOLERROR("Texture resource not found");
		}
	}
}