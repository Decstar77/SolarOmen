#include "SolarPrimitives.h"

namespace sol
{
	Ray Ray::Create(const Vec3f& origin, const Vec3f& direction)
	{
		Ray ray;

		ray.origin = origin;
		ray.direction = direction;

		return ray;
	}

	Vec3f Ray::TravelDown(const Ray& ray, const real32& distance)
	{
		Vec3f point = ray.origin + distance * ray.direction;

		return point;
	}

	Plane Plane::Create(const Vec3f& center, const Vec3f& normal)
	{
		Plane plane;

		plane.center = center;
		plane.normal = Normalize(normal);

		return plane;
	}


	bool8 Raycast::CheckPlane(const Ray& ray, const Plane& plane, RaycastInfo* info)
	{
		bool result = false;
		real32 demon = Dot(ray.direction, plane.normal);

		if (!Equal(demon, 0.0f, EPSILON))
		{
			Vec3f pr = plane.center - ray.origin;
			real32 nume = Dot(pr, plane.normal);

			real32 t = nume / demon;

			info->t = t;
			info->normal = plane.normal;
			info->closePoint = Ray::TravelDown(ray, t);

			result = t > 0.0f;
		}

		return result;
	}
}