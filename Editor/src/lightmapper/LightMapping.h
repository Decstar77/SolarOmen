#pragma once

#include "../Core.h"


namespace sol
{
	class LightMapper
	{

	};

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

	typedef ImplicitSphere<real64> Sphere;
	typedef ImplicitSphere<real64> PreciseSphere;

	struct PreciseRay
	{
		Vec3d origin;
		Vec3d direction;

		inline static PreciseRay Create(const Vec3d& origin, const Vec3d& direction)
		{
			PreciseRay ray = {};
			ray.origin = origin;
			ray.direction = direction;
			return ray;
		}

		inline Vec3d TravelDown(real64 t) const {
			return origin + t * direction;
		}
	};

	template<typename T>
	struct HitRecord
	{
		Vec3<T> p;
		Vec3<T> normal;
		T t;

		bool8 frontFace;

		inline void SetFaceNormal(const PreciseRay& r, const Vec3<T>& outward_normal) {
			frontFace = Dot(r.direction, outward_normal) < 0;
			normal = frontFace ? outward_normal : static_cast<T>(-1.0) * outward_normal;
		}
	};

	typedef HitRecord<real64> PreciseHitRecord;

	class RayTracerCamera
	{
	public:
		RayTracerCamera() {};

		void Initialize(Vec3d lookfrom, Vec3d lookat, Vec3d vup, real64 yFovDeg, real64 aspectRatio)
		{
			auto theta = DegToRad(yFovDeg);
			auto h = Tan(theta / 2);
			auto viewport_height = 2.0 * h;
			auto viewport_width = aspectRatio * viewport_height;

			auto w = Normalize(lookfrom - lookat);
			auto u = Normalize(Cross(vup, w));
			auto v = Cross(w, u);

			origin = lookfrom;
			horizontal = viewport_width * u;
			vertical = viewport_height * v;
			lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - w;
		}

		PreciseRay GetRay(real64 s, real64 t) const {
			return PreciseRay::Create(origin, lower_left_corner + s * horizontal + t * vertical - origin);
		}

	private:
		Vec3d origin;
		Vec3d lower_left_corner;
		Vec3d horizontal;
		Vec3d vertical;
	};

	class RayTracerWorld
	{
	public:
		std::vector<PreciseSphere> objects;

		bool8 Trace(const PreciseRay& r, real64 tMin, real64 tMax, PreciseHitRecord* hitrecord)const;
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

		RayTracerCamera camera;
		std::vector<Vec4f> pixels;

		RayTracerWorld world;

		void Initialize(uint32 samples);
		void Shutdown();
		void Trace();


	};
}

