#pragma once
#include "../../Core.h"

namespace sol
{
	struct RayTracingRay
	{
		Vec3d origin;
		Vec3d direction;
		real64 time;
		inline static RayTracingRay Create(const Vec3d& origin, const Vec3d& direction)
		{
			RayTracingRay ray = {};
			ray.origin = origin;
			ray.direction = direction;
			return ray;
		}

		inline Vec3d TravelDown(real64 t) const {
			return origin + t * direction;
		}
	};


	struct HitRecord
	{
		Vec3d p;
		Vec3d normal;
		real64 t;
		real64 u;
		real64 v;
		bool8 frontFace;

		std::shared_ptr<class RayTracingMaterial> material;

		inline void SetFaceNormal(const RayTracingRay& r, const Vec3d& outward_normal) {
			frontFace = Dot(r.direction, outward_normal) < 0;
			normal = frontFace ? outward_normal : -1.0 * outward_normal;
		}
	};


	inline Vec3d RandomPointInUnitSphere()
	{
		while (true)
		{
			Vec3d p = {};
			p.x = RandomReal64(-1.0, 1.0);
			p.y = RandomReal64(-1.0, 1.0);
			p.z = RandomReal64(-1.0, 1.0);

			if (MagSqrd(p) >= 1) { continue; }

			return p;
		}
	}

	inline Vec3d RandomPointInUnitHemisphere(const Vec3d& normal)
	{
		Vec3d in_unit_sphere = RandomPointInUnitSphere();
		return (Dot(in_unit_sphere, normal) > 0.0) ? in_unit_sphere : -1.0 * in_unit_sphere;
	}

	inline Vec3d RandomVec3() { return Vec3d(RandomReal64(), RandomReal64(), RandomReal64()); }
	inline Vec3d RandomVec3(real64 min, real64 max) { return Vec3d(RandomReal64(min, max), RandomReal64(min, max), RandomReal64(min, max)); }
}