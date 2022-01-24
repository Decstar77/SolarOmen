#include "RayTracingMaterials.h"

namespace sol
{
	static bool8 NearZero(const Vec3d& e)
	{
		const auto s = 1e-8;
		return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
	}

	bool8 Lambertian::Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const
	{
		Vec3d scatterDir = rec.normal + Normalize(RandomPointInUnitSphere());

		if (NearZero(scatterDir)) { scatterDir = rec.normal; }


		*scattered = RayTracingRay::Create(rec.p, scatterDir);
		*attenuation = albedo->Value(rec.u, rec.v, rec.p);
		return true;
	}

	bool8 MetalMaterial::Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const
	{
		Vec3d reflected = Reflect(Normalize(r.direction), rec.normal);
		*scattered = RayTracingRay::Create(rec.p, reflected + fuzz * RandomPointInUnitSphere());
		*attenuation = albedo;
		return (Dot(scattered->direction, rec.normal) > 0);
	}

	bool8 sol::DielectricMaterial::Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const
	{
		*attenuation = Vec3d(1.0, 1.0, 1.0);
		real64 refraction_ratio = rec.frontFace ? (1.0 / ir) : ir;

		Vec3d unit_direction = Normalize(r.direction);
		real64 cos_theta = Min(Dot(-1.0 * unit_direction, rec.normal), 1.0);
		real64 sin_theta = Sqrt(1.0 - cos_theta * cos_theta);

		bool8 cannot_refract = refraction_ratio * sin_theta > 1.0;
		Vec3d direction;

		if (cannot_refract || Reflectance(cos_theta, refraction_ratio) > RandomReal64())
			direction = Reflect(unit_direction, rec.normal);
		else
			direction = Refract(unit_direction, rec.normal, refraction_ratio);

		*scattered = RayTracingRay::Create(rec.p, direction);
		return true;
	}
}