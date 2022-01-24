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

	class RayTracingBox : public RayTracingObject
	{
	public:
		PreciseOBB obb;
		std::shared_ptr<RayTracingMaterial> material;
	public:
		RayTracingBox() {};
		RayTracingBox(Vec3d extents, Vec3d position, std::shared_ptr<RayTracingMaterial> m) : obb(extents, position), material(m) {	};
		virtual bool8 Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const override;
		virtual bool8 GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const override;
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

		PixelCache() : colour(0), samples(0), totalSamples(500), depth(50) {}
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

		std::vector<PixelCache> pixelCaches;
		std::vector<Vec4f> pixels;

		RayTracingWorld world;

		void Initialize(uint32 samples);
		void Shutdown();
		void Trace();


	};
}

