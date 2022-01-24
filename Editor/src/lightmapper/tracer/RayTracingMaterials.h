#pragma once
#include "RayTracingTextures.h"

namespace sol
{
	class RayTracingMaterial
	{
	public:
		virtual Vec3d Emission(real64 u, real64 v, const Vec3d& p) const { return Vec3d(0.0, 0.0, 0.0); }
		virtual bool8 Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const = 0;
	};

	class Lambertian : public RayTracingMaterial
	{
	public:
		Lambertian(const Vec3d& a) : albedo(std::make_shared<SolidColour>(a)) {}
		Lambertian(std::shared_ptr<RayTracingTexture> a) : albedo(a) {};
		virtual bool8 Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const override;
	private:
		std::shared_ptr<RayTracingTexture> albedo;
	};

	class MetalMaterial : public RayTracingMaterial
	{
	public:
		Vec3d albedo;
		real64 fuzz;
	public:
		MetalMaterial(const Vec3d& a, real64 f) : albedo(a), fuzz(f < 1 ? f : 1) {}
		virtual bool8 Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const override;
	};

	class DielectricMaterial : public RayTracingMaterial
	{
	public:
		real64 ir;

	public:
		DielectricMaterial(real64 index_of_refraction) : ir(index_of_refraction) {}
		virtual bool8 Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const override;

	private:
		inline real64 Reflectance(real64 cosine, real64 ref_idx) const {
			real64 r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
			r0 = r0 * r0;
			return r0 + (1.0 - r0) * Pow((1.0 - cosine), 5.0);
		}
	};

	class DiffuseLight : public RayTracingMaterial
	{
	public:
		DiffuseLight(std::shared_ptr<RayTracingTexture> a) : emit(a) {}
		DiffuseLight(Vec3d c) : emit(std::make_shared<SolidColour>(c)) {}
		virtual bool8 Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const override { return false; }
		virtual Vec3d Emission(real64 u, real64 v, const Vec3d& p) const { return emit->Value(u, v, p); }
	public:
		std::shared_ptr<RayTracingTexture> emit;
	};
}