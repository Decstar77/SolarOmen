#pragma once

#include "../Core.h"


namespace sol
{
	template<typename T>
	struct ImplicitSphere
	{
		Vec3<T> origin;
		T radius;

		inline static ImplicitSphere<T> Create(const Vec3<T>& origin, T radius)
		{
			ImplicitSphere sphere = {};
			sphere.origin = origin;
			sphere.radius = radius;
			return sphere;
		}
	};

	typedef ImplicitSphere<real32> Sphere;
	typedef ImplicitSphere<real64> PreciseSphere;

	template<typename T>
	struct ImplicitAABB
	{
		Vec3<T> min;
		Vec3<T> max;

		inline static ImplicitAABB<T> Create(const Vec3<T>& min, const Vec3<T>& max)
		{
			ImplicitAABB<T> aabb = {};
			aabb.min = min;
			aabb.max = max;
			return aabb;
		}
	};

	typedef ImplicitAABB<real32> AABB;
	typedef ImplicitAABB<real64> PreciseAABB;

	template<typename T>
	inline ImplicitAABB<T> SurroundingAABB(const ImplicitAABB<T>& box1, const ImplicitAABB<T>& box2)
	{
		ImplicitAABB<T> result = {};

		result.min = Min(box1.min, box2.min);
		result.max = Max(box1.max, box2.max);

		return result;
	}

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

	class RayTracingCamera
	{
	public:
		RayTracingCamera() {};

		void Initialize(Vec3d lookfrom, Vec3d lookat, Vec3d   vup, real64 vfov, real64 aspect_ratio, real64 aperture, real64 focus_dist);
		RayTracingRay GetRay(double s, double t) const;

		inline void SetTime(real64 t1, real64 t2) { time0 = t1; time1 = t2; };

	private:
		Vec3d origin;
		Vec3d lower_left_corner;
		Vec3d horizontal;
		Vec3d vertical;
		Vec3d u, v, w;
		real64 lens_radius;
		real64 time0;
		real64 time1;
	};



	class RayTracingTexture
	{
	public:
		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const = 0;
	};

	class SolidColour : public RayTracingTexture
	{
	public:
		SolidColour() {};
		SolidColour(const Vec3d& colour) : colour(colour) {}
		SolidColour(real64 r, real64 g, real64 b) : colour(r, g, b) {};

		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const override { return colour; };

	private:
		Vec3d colour;
	};

	class CheckerTexture : public RayTracingTexture
	{
	public:
		CheckerTexture() {};
		CheckerTexture(const Vec3d& colour1, const Vec3d& colour2)
			: even(std::make_shared<SolidColour>(colour1)), odd(std::make_shared<SolidColour>(colour2)) {}

		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const override;
	public:
		std::shared_ptr<SolidColour> odd;
		std::shared_ptr<SolidColour> even;
	};



	class RayTracingMaterial
	{
	public:
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
			return r0 + (1.0 - r0) * Pow<real64>((1.0 - cosine), 5.0);
		}
	};



	class RayTracingObject
	{
	public:
		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const = 0;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const = 0;
	};

	class RayTracingSphere : public RayTracingObject
	{
	public:
		PreciseSphere sphere;
		std::shared_ptr<RayTracingMaterial> material;

	public:
		RayTracingSphere() {};
		RayTracingSphere(Vec3d origin, real64 r, std::shared_ptr<RayTracingMaterial> m) : sphere(PreciseSphere::Create(origin, r)), material(m) {};
		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;
		void GetUV(const Vec3d& p, real64* u, real64* v) const;
	};

	class RayTracingBVHNode : public RayTracingObject
	{
	public:
		RayTracingBVHNode() {};

		bool8 Build(const std::vector<std::shared_ptr<RayTracingObject>>& srcObjects, uint64 start, uint64 end, real64 time0, real64 time1);

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* hitrecord) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

	public:
		std::shared_ptr<RayTracingObject> left;
		std::shared_ptr<RayTracingObject> right;
		PreciseAABB nodeBox;
	};

	class RayTracingWorld : public RayTracingObject
	{
	public:
		std::vector<std::shared_ptr<RayTracingObject>> objects;
		RayTracingBVHNode bvhTree;

		void MakeRandomSphereWorld();
		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* hitrecord) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;
	};

	class ReferenceRayTracer
	{
	public:
		int32 imageWidth;
		int32 imageHeight;
		real64 aspectRatio;

		uint32 pixelsProcessed;
		uint32 samplesPerPixel;

		uint32 updateCount;
		TextureHandle textureHandle;
		bool8 complete;

		RayTracingCamera camera;
		std::vector<Vec4f> pixels;

		RayTracingWorld world;

		void Initialize(uint32 samples);
		void Shutdown();
		void Trace();


	};
}

