#pragma once

#include "../Core.h"
#include "tracer/RayTracingRay.h"
#include "tracer/RayTracingCamera.h"
#include "tracer/RayTracingTextures.h"
#include "tracer/RayTracingMaterials.h"

namespace sol
{
	class RayTracingObject
	{
	public:
		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const = 0;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const = 0;
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

	class RayTracingHittableList : public RayTracingObject
	{
	public:
		RayTracingHittableList() {};
		inline void Open() { objects.clear(); bvhTree = RayTracingBVHNode(); };
		inline void Add(std::shared_ptr<RayTracingObject> object) { objects.push_back(object); }
		inline void Seal() { bvhTree.Build(objects, 0, objects.size(), 0, 0); }

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

	private:
		RayTracingBVHNode bvhTree;
		std::vector<std::shared_ptr<RayTracingObject>> objects;
	};

	class RayTracingSphere : public RayTracingObject
	{
	public:
		PreciseSphere sphere;
		std::shared_ptr<RayTracingMaterial> material;

	public:
		RayTracingSphere() {};
		RayTracingSphere(Vec3d origin, real64 r, std::shared_ptr<RayTracingMaterial> m) : sphere(origin, r), material(m) {};
		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;
		void GetUV(const Vec3d& p, real64* u, real64* v) const;
	};

	class  RayTracingXYRect : public RayTracingObject
	{
	public:
		RayTracingXYRect() : x0(0), x1(0), y0(0), y1(0), k(0) {}
		RayTracingXYRect(real64 _x0, real64 _x1, real64 _y0, real64 _y1, real64 _k, std::shared_ptr<RayTracingMaterial> mat)
			: x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), material(mat) {};

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

	public:
		std::shared_ptr<RayTracingMaterial> material;
		real64 x0, x1, y0, y1, k;
	};

	class RayTracingXZRect : public RayTracingObject
	{
	public:
		RayTracingXZRect() : x0(0), x1(0), z0(0), z1(0), k(0) {}
		RayTracingXZRect(real64 _x0, real64 _x1, real64 _z0, real64 _z1, real64 _k, std::shared_ptr<RayTracingMaterial> mat)
			: x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), material(mat) {};

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

	public:
		std::shared_ptr<RayTracingMaterial> material;
		real64 x0, x1, z0, z1, k;
	};

	class RayTracingYZRect : public RayTracingObject
	{
	public:
		RayTracingYZRect() : y0(0), y1(0), z0(0), z1(0), k(0) {}
		RayTracingYZRect(real64 _y0, real64 _y1, real64 _z0, real64 _z1, real64 _k, std::shared_ptr<RayTracingMaterial> mat)
			: y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), material(mat) {};

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

	public:
		std::shared_ptr<RayTracingMaterial> material;
		real64 y0, y1, z0, z1, k;
	};

	class RayTracingBox : public RayTracingObject
	{
	public:
		RayTracingBox() {};
		RayTracingBox(Vec3d boxMin, Vec3d boxMax, std::shared_ptr<RayTracingMaterial> m);
		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

	public:
		Vec3d boxMin;
		Vec3d boxMax;
		RayTracingHittableList sides;
		std::shared_ptr<RayTracingMaterial> material;
	};

	class RayTracingTranslate : public RayTracingObject
	{
	public:
		RayTracingTranslate(std::shared_ptr<RayTracingObject> p, const Vec3d& displacement) : ptr(p), offset(displacement) {}

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

	public:
		std::shared_ptr<RayTracingObject> ptr;
		Vec3d offset;
	};

	class RayTracingRotateY : public RayTracingObject
	{
	public:
		RayTracingRotateY(std::shared_ptr<RayTracingObject> p, real64 angle);

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override {
			*box = bbox;
			return hasbox;
		};

	public:
		std::shared_ptr<RayTracingObject> ptr;
		real64 sin_theta;
		real64 cos_theta;
		bool8 hasbox;
		PreciseAABB bbox;
	};

	class RayTracingWorld : public RayTracingObject
	{
	public:
		RayTracingHittableList objects;
		Vec3d backgroundColour;

		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* hitrecord) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;

		void MakeRandomSphereWorld(RayTracingCamera* camera, real64 aspectRatio);
		void MakeTwoSphereWorld(RayTracingCamera* camera, real64 aspectRatio);
		void MakeTwoPerlineSpheres(RayTracingCamera* camera, real64 aspectRatio);
		void MakeTextureWorld(RayTracingCamera* camera, real64 aspectRatio);
		void MakeSimpleLight(RayTracingCamera* camera, real64 aspectRatio);
		void MakeBoxWorld(RayTracingCamera* camera, real64 aspectRatio);
		void MakeCornellBox(RayTracingCamera* camera, real64 aspectRatio);
	};

	class PixelCache
	{
	public:
		Vec3d colour;
		int32 samples;
		int32 totalSamples;
		int32 depth;

		PixelCache() : colour(0), samples(0), totalSamples(10), depth(1) {}
	};

	class ReferenceRayTracer
	{
	public:
		int32 imageWidth;
		int32 imageHeight;
		real64 aspectRatio;


		uint32 samplesPerPixel;

		uint32 updateCount;
		TextureHandle textureHandle;
		bool8 complete;

		RayTracingCamera camera;

		std::vector<PixelCache> pixelCaches;
		std::vector<Vec4f> pixels;

		RayTracingWorld world;

		void Initialize(uint32 samples);
		void Initialize(const Camera& camera);
		void Shutdown();
		void Trace();


	};
}

