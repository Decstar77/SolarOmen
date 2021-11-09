#include "SimpleColliders.h"


namespace cm
{
	struct OverlapInterval
	{
		real32 min;
		real32 max;
	};

	inline static OverlapInterval GetOverlapInterval(const AABB& aabb, const Vec3f& axis)
	{
		Vec3f vertices[8] = { aabb.min, aabb.max,
							 Vec3f(aabb.max.x, aabb.min.y, aabb.min.z),
							 Vec3f(aabb.max.x, aabb.max.y, aabb.min.z),
							 Vec3f(aabb.min.x, aabb.max.y, aabb.min.z),
							 Vec3f(aabb.max.x, aabb.min.y, aabb.max.z),
							 Vec3f(aabb.min.x, aabb.min.y, aabb.max.z),
							 Vec3f(aabb.min.x, aabb.max.y, aabb.max.z) };

		OverlapInterval result;
		result.min = Dot(axis, vertices[0]);
		result.max = result.min;

		for (int32 i = 1; i < 8; i++)
		{
			real32 projection = Dot(axis, vertices[i]);

			result.min = (projection < result.min) ? projection : result.min;
			result.max = (projection > result.max) ? projection : result.max;
		}

		return result;
	}

	inline static OverlapInterval GetOverlapInterval(const OBB& obb, const Vec3f& axis)
	{
		Vec3f vertices[8];

		vertices[0] = obb.center + (obb.basis.right * obb.extents.x) + (obb.basis.upward * obb.extents.y) + (obb.basis.forward * obb.extents.z);
		vertices[1] = obb.center - (obb.basis.right * obb.extents.x) + (obb.basis.upward * obb.extents.y) + (obb.basis.forward * obb.extents.z);
		vertices[2] = obb.center + (obb.basis.right * obb.extents.x) - (obb.basis.upward * obb.extents.y) + (obb.basis.forward * obb.extents.z);
		vertices[3] = obb.center + (obb.basis.right * obb.extents.x) + (obb.basis.upward * obb.extents.y) - (obb.basis.forward * obb.extents.z);
		vertices[4] = obb.center - (obb.basis.right * obb.extents.x) - (obb.basis.upward * obb.extents.y) - (obb.basis.forward * obb.extents.z);
		vertices[5] = obb.center + (obb.basis.right * obb.extents.x) - (obb.basis.upward * obb.extents.y) - (obb.basis.forward * obb.extents.z);
		vertices[6] = obb.center - (obb.basis.right * obb.extents.x) + (obb.basis.upward * obb.extents.y) - (obb.basis.forward * obb.extents.z);
		vertices[7] = obb.center - (obb.basis.right * obb.extents.x) - (obb.basis.upward * obb.extents.y) + (obb.basis.forward * obb.extents.z);

		OverlapInterval result;
		result.min = Dot(axis, vertices[0]);
		result.max = result.min;

		for (int32 i = 1; i < 8; i++)
		{
			real32 projection = Dot(axis, vertices[i]);

			result.min = (projection < result.min) ? projection : result.min;
			result.max = (projection > result.max) ? projection : result.max;
		}

		return result;
	}

	inline static bool DoPrimitivesOverlapOnAxis(const AABB& aabb, const OBB& obb, const Vec3f& axis)
	{
		OverlapInterval a = GetOverlapInterval(aabb, axis);
		OverlapInterval b = GetOverlapInterval(obb, axis);

		bool result = (b.min <= a.max) && (a.min <= b.max);

		return result;
	}

	inline static bool DoPrimitivesOverlapOnAxis(const OBB& a, const OBB& b, const Vec3f& axis)
	{
		OverlapInterval ai = GetOverlapInterval(a, axis);
		OverlapInterval bi = GetOverlapInterval(b, axis);

		bool result = (bi.min <= ai.max) && (ai.min <= bi.max);

		return result;
	}

	inline static OverlapInterval GetOverlapInterval(const Triangle& triangle, const Vec3f& axis)
	{
		real32 v0 = Dot(triangle.v0, axis);
		real32 v1 = Dot(triangle.v1, axis);
		real32 v2 = Dot(triangle.v2, axis);

		OverlapInterval result;

		result.min = Min(v0, v1, v2);
		result.max = Max(v0, v1, v2);

		return result;
	}

	inline static bool DoPrimitivesOverlapOnAxis(const Triangle& triangle, const AABB& aabb, const Vec3f& axis)
	{
		OverlapInterval box = GetOverlapInterval(aabb, axis);
		OverlapInterval tri = GetOverlapInterval(triangle, axis);

		bool result = (tri.min <= box.max) && (box.min <= tri.max);

		return result;
	}

	inline static bool DoPrimitivesOverlapOnAxis(const Triangle& triangle, const OBB& obb, const Vec3f& axis)
	{
		OverlapInterval a = GetOverlapInterval(obb, axis);
		OverlapInterval b = GetOverlapInterval(triangle, axis);

		bool result = (b.min <= a.max) && (a.min <= b.max);

		return result;
	}

	inline static bool DoPrimitivesOverlapOnAxis(const Triangle& t1, const Triangle& t2, const Vec3f& axis)
	{
		OverlapInterval a = GetOverlapInterval(t1, axis);
		OverlapInterval b = GetOverlapInterval(t2, axis);

		bool result = (b.min <= a.max) && (a.min <= b.max);

		return result;
	}

	bool RaycastPlane(const Ray& ray, const Plane& plane, real32* dist)
	{
		bool result = false;
		real32 demon = Dot(ray.direction, plane.normal);

		if (!Equal(demon, 0.0f, Intersection_ERROR_TOLLERANCE))
		{
			Vec3f pr = plane.center - ray.origin;
			real32 nume = Dot(pr, plane.normal);

			real32 t = nume / demon;

			result = t >= 0.0f;

			if (dist)
			{
				*dist = t;
			}
		}

		return result;
	}

	bool RaycastSphere(const Ray& ray, const Sphere& sphere, real32* dist)
	{
		real32 rad = GetSphereRadius(sphere);
		Vec3f rs = ray.origin - GetSphereCenter(sphere);

		real32 a = Dot(ray.direction, ray.direction);
		real32 b = 2.0f * Dot(rs, ray.direction);
		real32 c = Dot(rs, rs) - rad * rad;
		real32 d = b * b - 4.0f * a * c;

		bool result = (d > 0.00001f);

		real32 t = -1.0f;

		if (result)
		{
			t = (-b - Sqrt(d)) / (2.0f * a);
		}

		result = (t > 0.00001f);

		if (dist)
		{
			*dist = t;
		}

		return result;
	}

	bool RaycastAABB(const Ray& ray, const AABB& aabb, real32* dist)
	{
		if (IsPointInsideAABB(aabb, ray.origin))
			return true;

		Vec3f min = (aabb.min - ray.origin) / ray.direction;
		Vec3f max = (aabb.max - ray.origin) / ray.direction;

		if (min.x > max.x)
		{
			Swap(&min.x, &max.x);
		}

		if (min.y > max.y)
		{
			Swap(&min.y, &max.y);
		}

		if (min.z > max.z)
		{
			Swap(&min.z, &max.z);
		}

		bool result = false;
		real32 tmin = min.x;
		real32 tmax = max.x;

		if ((tmin < max.y) && (min.y < tmax))
		{
			tmin = (min.y > tmin) ? min.y : tmin;
			tmax = (max.y < tmax) ? max.y : tmax;

			if ((tmin < max.z) && (min.z < tmax))
			{
				tmin = (min.z > tmin) ? min.z : tmin;
				tmax = (max.z < tmax) ? max.z : tmax;

				result = (tmin > 0);

				if (dist)
				{
					// @TODO:
					*dist = tmin;
				}
			}
		}

		return result;
	}

	bool RaycastTriangle(const Ray& ray, const Triangle& triangle, real32* dist)
	{
		Plane plane = CreatePlane(triangle);
		bool result = false;

		real32 t;
		if (RaycastPlane(ray, plane, &t))
		{
			Vec3f point = TravelDownRay(ray, t);
			Vec3f bar = GetBarycentricTriangle(triangle, point);

			if (bar.x >= 0.0f && bar.x <= 1.0f &&
				bar.y >= 0.0f && bar.y <= 1.0f &&
				bar.z >= 0.0f && bar.z <= 1.0f) // TODO: Make a nice function to test if barycentric coords are normailed/valid/homogenous/correct ?
			{
				result = true;
				if (dist)
				{
					*dist = t;
				}
			}
		}

		return result;
	}

	bool RaycastMeshColldier(const Ray& ray, const MeshCollider& mesh_collider, real32* dist)
	{
		Assert(mesh_collider.size() > 0, "INVALID MESH COLLDIER");

		bool result = false;

		if (dist)
		{
			real32 min = FLT_MAX;
			for (int32 i = 0; i < (int32)mesh_collider.size(); i++)
			{
				Triangle tri = mesh_collider.at(i);

				real32 t = FLT_MAX;
				result = result || RaycastTriangle(ray, tri, &t);

				min = t < min ? t : min;
			}

			*dist = min;
		}
		else
		{
			for (int32 i = 0; i < (int32)mesh_collider.size(); i++)
			{
				Triangle tri = mesh_collider.at(i);

				result = RaycastTriangle(ray, tri);

				if (result)
				{
					break;
				}
			}
		}

		return result;
	}

	bool RaycastOBB(const Ray& ray, const OBB& obb, real32* dist)
	{
		real32 t[6] = {}; // tx_min, tx_max, ty_min, ty_max, tz_min, tz_max
		Vec3f to_origin = obb.center - ray.origin;
		bool result = true;

		for (int32 i = 0; (i < 3) && result; i++)
		{
			real32 demon = Dot(ray.direction, obb.basis.mat[i]);
			real32 r = Dot(to_origin, obb.basis.mat[i]);

			if (Abs(demon) < 0.001f)
			{
				// @NOTE: Ray parallel to a plane
				Vec3f to_origin = obb.center - ray.origin;

				if (-r - obb.extents[i] > 0 || -r + obb.extents[i] < 0)
				{
					result = false;
					break;
				}

				demon = 0.001f;
			}

			t[i * 2 + 0] = (r + obb.extents[i]) / demon;
			t[i * 2 + 1] = (r - obb.extents[i]) / demon;
		}

		if (result)
		{
			real32 tx_min = Min(t[0], t[1]);
			real32 ty_min = Min(t[2], t[3]);
			real32 tz_min = Min(t[4], t[5]);

			real32 tmin = Max(tx_min, ty_min, tz_min);

			real32 tx_max = Max(t[0], t[1]);
			real32 ty_max = Max(t[2], t[3]);
			real32 tz_max = Max(t[4], t[5]);

			real32 tmax = Min(tx_max, ty_max, tz_max);

			// @TODO: if tmin < 0 we started in side obb, ie pen is tmax else tmin
			result = (tmax > 0) && (tmin < tmax);
			if (dist)
			{
				*dist = tmin;
			}
		}

		return result;
	}

	bool CheckIntersectionLineTriangle(const LineSegment& line, const Triangle& triangle)
	{
		Ray ray = CreateRay(line.start, Normalize(line.end - line.start));

		real32 t;
		bool result = RaycastTriangle(ray, triangle, &t);

		result = (result) ? (t >= 0.0f && t * t < GetLineSegmentLengthSqrd(line)) : false;

		return result;
	}

	bool CheckIntersectionCapsulePlane(const Capsule& capsule, const Plane& plane)
	{
		LineSegment line = CreateLineSegment(capsule.bot, capsule.top);

		bool result = CheckIntersectionLinePlane(line, plane);

		if (!result)
		{
			real32 r2 = capsule.radius * capsule.radius;

			Vec3f e1 = ClosestPointOnPlane(plane, capsule.bot);
			Vec3f e2 = ClosestPointOnPlane(plane, capsule.top);

			Vec3f f1 = ClosestPointOnLineSegment(line, e1);
			Vec3f f2 = ClosestPointOnLineSegment(line, e2);

			real32 d1 = DistanceSqrd(e1, f1);
			real32 d2 = DistanceSqrd(e2, f2);

			Vec3f sc = (d1 < d2) ? f1 : f2;
			// @NOTE: For simplicity/understanability sake I've left it to the general algorithm
			//		: however we recompute a few things, such as closest point to plane, so we
			//		: can do the "sphere intersection" test in place (the else case) for speeeds
#if 0
			Sphere s = CreateSphere(sc, capsule.radius);

			result = CheckIntersectionPlaneSphere(plane, s);
#else
			Vec3f pp = (d1 < d2) ? e1 : e2;

			real32 d3 = DistanceSqrd(sc, pp);

			result = d3 <= r2;
#endif
		}

		return result;
	}

	bool CheckIntersectionAABBSphere(const AABB& aabb, const Sphere& sphere)
	{
		real32 sr = GetSphereRadius(sphere);
		Vec3f sc = GetSphereCenter(sphere);

		Vec3f close = ClosestPointOnAABB(aabb, sc);

		real32 distance2 = DistanceSqrd(close, sc);

		bool result = distance2 <= (sr * sr);

		return result;
	}

	bool CheckIntersectionOBBSphere(const OBB& obb, const Sphere& sphere)
	{
		real32 sr = GetSphereRadius(sphere);
		Vec3f sc = GetSphereCenter(sphere);

		Vec3f close = ClosestPointOnOBB(obb, sc);

		real32 distance2 = DistanceSqrd(close, sc);

		bool result = distance2 <= (sr * sr);

		return result;
	}

	bool CheckIntersectionPlaneSphere(const Plane& plane, const Sphere& sphere)
	{
		Vec3f sc = GetSphereCenter(sphere);
		real32 sr = GetSphereRadius(sphere);

		Vec3f close = ClosestPointOnPlane(plane, sc);

		real32 dist2 = DistanceSqrd(close, sc);

		bool result = dist2 <= (sr * sr);

		return result;
	}

	bool CheckIntersectionAABBOBB(const AABB& aabb, const OBB& obb)
	{
		Vec3f axises[15] = {};
		axises[0] = Vec3f(1, 0, 0);	   // @NOTE: AABB right
		axises[1] = Vec3f(0, 1, 0);	   // @NOTE: AABB upward
		axises[2] = Vec3f(0, 0, 1);	   // @NOTE: AABB forward
		axises[3] = obb.basis.right;   // @NOTE: OBB right
		axises[4] = obb.basis.upward;  // @NOTE: OBB upward
		axises[5] = obb.basis.forward; // @NOTE: OBB forward

		axises[6] = Cross(axises[0], axises[3]);
		axises[7] = Cross(axises[0], axises[4]);
		axises[8] = Cross(axises[0], axises[5]);

		axises[9] = Cross(axises[1], axises[3]);
		axises[10] = Cross(axises[1], axises[4]);
		axises[11] = Cross(axises[1], axises[5]);

		axises[12] = Cross(axises[2], axises[3]);
		axises[13] = Cross(axises[2], axises[4]);
		axises[14] = Cross(axises[2], axises[5]);

		bool result = true;

		for (int32 i = 0; result && i < 15; i++)
		{
			if (!DoPrimitivesOverlapOnAxis(aabb, obb, axises[i]))
			{
				result = false;
			}
		}

		return result;
	}

	bool CheckIntersectionAABBPlane(const AABB& aabb, const Plane& plane)
	{
		Vec3f c = GetAABBCenter(aabb);
		Vec3f e = GetAABBRadius(aabb);

		// @NOTE: The length of projecting the aabb extents onto a ray defined by the plane normal.
		// 		: Beacause the aabb is axis aligned we just take the plane normal's components
		//		: In other worlds it's actually Abs(Dot(plane.normal, Vec3(1, 0, 0))) which is just
		//		: plane.normal.x. Have a lookt at OBBPlane where this dot actaully matters. The Vec3(1,0,0)
		//		: represents the aabb/objects orientation. Ie axis aligned.
		real32 r = e.x * Abs(plane.normal.x) + e.y * Abs(plane.normal.y) + e.z * Abs(plane.normal.z);

		real32 d = GetPlaneScalar(plane);
		real32 t = Dot(plane.normal, c) - d;

		bool result = Abs(t) <= r; //@NOTE: -r <= t <= r

		return result;
	}

	bool CheckIntersectionOBBPlane(const OBB& obb, const Plane& plane)
	{
		// @NOTE: The length of projecting the aabb extents onto a ray defined by the plane normal.
		real32 r = obb.extents.x * Abs(Dot(plane.normal, obb.basis.right)) +
			obb.extents.y * Abs(Dot(plane.normal, obb.basis.upward)) +
			obb.extents.z * Abs(Dot(plane.normal, obb.basis.forward));

		real32 d = GetPlaneScalar(plane);
		real32 t = Dot(plane.normal, obb.center) - d;

		bool result = Abs(t) <= r; //@NOTE: -r <= t <= r

		return result;
	}

	bool CheckIntersectionTrianglePlane(const Triangle& triangle, const Plane& plane)
	{
		Assert(0, "CheckIntersectionTrianglePlane untested");

		Vec3f a = triangle.v0 - plane.center;
		Vec3f b = triangle.v1 - plane.center;
		Vec3f c = triangle.v2 - plane.center;

		real32 aa = Dot(a, plane.normal);
		real32 bb = Dot(b, plane.normal);
		real32 cc = Dot(c, plane.normal);

		real32 min = Min(aa, bb, cc);
		real32 max = Max(aa, bb, cc);

		bool result = min <= 0 && max >= 0;

		return result;
	}

	bool CheckIntersectionPlane(const Plane& a, const Plane& b)
	{
		// @NOTE: If c is 0 then planes are parallel
		Vec3f c = Cross(a.normal, b.normal);
		// @NOTE: This dot is just for conviece
		real32 dot = Dot(c, c);

		bool result = Equal(dot, 0.0f, Intersection_ERROR_TOLLERANCE);

		return result;
	}

	bool CheckIntersectionSphere(const Sphere& a, const Sphere& b)
	{
		real32 t = (a.data.w + b.data.w) * (a.data.w + b.data.w);

		Vec3f ab = Vec3f(a.data.x - b.data.x,
			a.data.y - b.data.y,
			a.data.z - b.data.z);

		real32 dist2 = MagSqrd(ab);

		bool result = dist2 <= t;

		return result;
	}

	bool CheckIntersectionAABB(const AABB& a, const AABB& b)
	{
		bool x = (a.max.x > b.min.x && a.min.x < b.max.x);
		bool y = (a.max.y > b.min.y && a.min.y < b.max.y);
		bool z = (a.max.z > b.min.z && a.min.z < b.max.z);

		bool result = x && y && z;

		return result;
	}

	bool CheckIntersectionOBB(const OBB& a, const OBB& b)
	{
		Vec3f axises[15] = {};

		axises[0] = a.basis.right;
		axises[1] = a.basis.upward;
		axises[2] = a.basis.forward;

		axises[3] = b.basis.right;
		axises[4] = b.basis.upward;
		axises[5] = b.basis.forward;

		axises[6] = Cross(axises[0], axises[3]);
		axises[7] = Cross(axises[0], axises[4]);
		axises[8] = Cross(axises[0], axises[5]);

		axises[9] = Cross(axises[1], axises[3]);
		axises[10] = Cross(axises[1], axises[4]);
		axises[11] = Cross(axises[1], axises[5]);

		axises[12] = Cross(axises[2], axises[3]);
		axises[13] = Cross(axises[2], axises[4]);
		axises[14] = Cross(axises[2], axises[5]);

		bool result = true;

		for (int32 i = 0; result && i < 15; i++)
		{
			if (!DoPrimitivesOverlapOnAxis(a, b, axises[i]))
			{
				result = false;
			}
		}

		return result;
	}

	bool CheckIntersectionLineSphere(const LineSegment& line, const Sphere& sphere)
	{
		Vec3f c = GetSphereCenter(sphere);

		Vec3f close = ClosestPointOnLineSegment(line, c);
		real32 d2 = DistanceSqrd(close, c);

		bool result = d2 <= sphere.data.w * sphere.data.w;

		return result;
	}

	bool CheckIntersectionLineAABB(const LineSegment& line, const AABB& aabb)
	{
		Assert(0, "Not implemented CheckIntersectionLineAABB");
		return false;
		// 		   Ray ray;
		//    ray.origin = line.start;
		//    ray.direction = Normalized(line.end - line.start);
		//    float t = Raycast(aabb, ray);

		//    return t >= 0 && t * t <= LengthSq(line);
	}

	bool CheckIntersectionTriangleSphere(const Triangle& triangle, const Sphere& sphere)
	{
		Vec3f sc = GetSphereCenter(sphere);
		real32 sr = GetSphereRadius(sphere);
		real32 sr2 = sr * sr;

		Vec3f close = ClosestPointOnTriangle(triangle, sc);

		real32 d2 = DistanceSqrd(close, sc);

		bool result = d2 <= sr2;

		return result;
	}

	bool CheckIntersectionTriangleAABB(const Triangle& triangle, const AABB& aabb)
	{
		Vec3f f0 = triangle.v1 - triangle.v0;
		Vec3f f1 = triangle.v2 - triangle.v1;
		Vec3f f2 = triangle.v0 - triangle.v2;

		Vec3f u0 = Vec3f(1, 0, 0);
		Vec3f u1 = Vec3f(0, 1, 0);
		Vec3f u2 = Vec3f(0, 0, 1);

		Vec3f axises[13] = {
			u0,
			u1,
			u2,
			Cross(u0, f0),
			Cross(u0, f1),
			Cross(u0, f2),
			Cross(u1, f0),
			Cross(u1, f1),
			Cross(u1, f2),
			Cross(u2, f0),
			Cross(u2, f1),
			Cross(u2, f2) };

		bool result = true;

		for (int32 i = 0; result && i < 13; i++)
		{
			if (!DoPrimitivesOverlapOnAxis(triangle, aabb, axises[i]))
			{
				result = false;
			}
		}

		return result;
	}

	bool CheckIntersectionTriangleOBB(const Triangle& triangle, const OBB& obb)
	{
		Vec3f f0 = triangle.v1 - triangle.v0;
		Vec3f f1 = triangle.v2 - triangle.v1;
		Vec3f f2 = triangle.v0 - triangle.v2;

		Vec3f u0 = obb.basis.right;
		Vec3f u1 = obb.basis.upward;
		Vec3f u2 = obb.basis.forward;

		Vec3f axises[13] = {
			u0,
			u1,
			u2,
			Cross(u0, f0),
			Cross(u0, f1),
			Cross(u0, f2),
			Cross(u1, f0),
			Cross(u1, f1),
			Cross(u1, f2),
			Cross(u2, f0),
			Cross(u2, f1),
			Cross(u2, f2) };

		bool result = true;

		for (int32 i = 0; result && i < 13; i++)
		{
			if (!DoPrimitivesOverlapOnAxis(triangle, obb, axises[i]))
			{
				result = false;
			}
		}

		return result;
	}

	bool CheckIntersectionTriangle(const Triangle& a, const Triangle b)
	{
		Assert(0, "Robustness of the Separating Axis Theorem");

		Vec3f t1e0 = a.v1 - a.v0; // @NOTE: Triangle 1, Edge 0
		Vec3f t1e1 = a.v2 - a.v1; // @NOTE: Triangle 1, Edge 1
		Vec3f t1e2 = a.v0 - a.v2; // @NOTE: Triangle 1, Edge 2

		Vec3f t2e0 = b.v1 - b.v0; // @NOTE: Triangle 2, Edge 0
		Vec3f t2e1 = b.v2 - b.v1; // @NOTE: Triangle 2, Edge 1
		Vec3f t2e2 = b.v0 - b.v2; // @NOTE: Triangle 2, Edge 2

		Vec3f axises[11] = {
			Cross(t1e0, t1e1),
			Cross(t2e0, t2e1),

			Cross(t2e0, t1e0),
			Cross(t2e0, t1e1),
			Cross(t2e0, t1e2),

			Cross(t2e1, t1e0),
			Cross(t2e1, t1e1),
			Cross(t2e1, t1e2),

			Cross(t2e2, t1e0),
			Cross(t2e2, t1e1),
			Cross(t2e2, t1e2) };

		bool result = true;

		for (int32 i = 0; result && i < 11; ++i)
		{
			if (!DoPrimitivesOverlapOnAxis(a, b, axises[i]))
			{
				result = false;
			}
		}

		return result;
	}

	bool CheckIntersectionLinePlane(const LineSegment& line, const Plane& plane)
	{
		Vec3f ab = line.end - line.start;

		real32 nume = Dot(plane.normal, line.start);
		real32 demon = Dot(plane.normal, ab);

		bool result = false;

		if (!Equal(demon, 0.0f, Intersection_ERROR_TOLLERANCE))
		{
			real32 d = GetPlaneScalar(plane);

			real32 t = (d - nume) / demon;

			result = (t >= 0.0f) && (t <= 1.0f);
		}

		return result;
	}

	bool CheckIntersectionLineOBB(const LineSegment& line, const OBB& obb)
	{
		Assert(0, "Not implemented CheckIntersectionLineOBB");
		return false;

		// 		   Ray ray;
		//    ray.origin = line.start;
		//    ray.direction = Normalized(line.end - line.start);
		//    float t = Raycast(obb, ray);

		//    return t >= 0 && t * t <= LengthSq(line);
	}
}