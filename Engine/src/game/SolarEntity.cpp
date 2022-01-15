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
		return "";
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

	void Entity::SetParent(Entity* entity)
	{
		Assert(IsValid(), "Entity invalid");
		Assert(this->id != entity->id, "Cannot parent an entity to its self !!");

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
		if (Entity* newParent = entity->id.Get())
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

			*entity = *newParent;
		}
		else
		{
			stored->parent = {};
			stored->siblingBehind = {};
			stored->siblingAhead = {};
		}

		*this = *stored;
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

	MaterialComponent* Entity::GetMaterialomponent()
	{
		Assert(IsValid(), "Entity invalid");
		return &room->materialComponets[id.index];
	}

	void Entity::SetMaterial(const String& modelName, const String& textureName)
	{
		Assert(IsValid(), "Entity invalid");
		ModelResource* model = Resources::GetModelResource(modelName);
		//TextureResource* texture = Resources::GetTextureResource(modelName);

		if (model)
		{
			room->materialComponets[id.index].material.modelId = model->id;
		}
		else
		{
			SOLERROR("Model resource not found");
		}

		/*	if (texture)
			{
				room->materialComponets[id.index].material.albedoId = texture->id;
			}
			else
			{
				SOLERROR("Texture resource not found");
			}*/
	}
}