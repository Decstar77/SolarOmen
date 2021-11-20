#pragma once

#include "SimpleColliders.h"

namespace cm
{
	struct RaycastInfo
	{
		// @NOTE: Going to remove these at some point
		Vec3f closePoint;
		Vec3f farPoint;
		Vec3f normal;
		real32 t;
	};

	struct Manifold
	{
		Vec3f normal;
		Vec3f pt_onA_World;
		Vec3f pt_onA_Body;
		Vec3f pt_onB_World;
		Vec3f pt_onB_Body;

		// @TODO: REMOVE!!!
		int32 bodyAIndex;
		int32 bodyBIndex;

		real32 seperationDistance;
		real32 toi;
	};

	bool RaycastPlane(const Ray& ray, const Plane& plane, RaycastInfo* info);

	bool RaycastSphere(const Ray& ray, const Sphere& sphere, RaycastInfo* info);

	bool RaycastAABB(const Ray& ray, const AABB& aabb, RaycastInfo* info);

	bool RaycastMeshCollider(const Ray& ray, const ManagedArray<Triangle>& triangles, RaycastInfo* info);

	bool CheckManifoldSphere(const Sphere& a, const Sphere& b, Manifold* info);

	bool CheckManifoldSphereOBB(const Sphere& sphere, const OBB& obb, Manifold* info);

	bool CheckManifoldAABB(const AABB& a, const AABB& b, Manifold* info);

	bool SweepManifoldSphere(const Sphere& sA, const Vec3f& velA, const Sphere& sB, const Vec3f& velB, real32 dt, Manifold* info);
}


