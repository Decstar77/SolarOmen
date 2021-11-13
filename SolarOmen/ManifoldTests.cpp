#include "ManifoldTests.h"


namespace cm
{
	bool RaycastPlane(const Ray& ray, const Plane& plane, RaycastInfo* info)
	{
		bool result = false;
		real32 demon = Dot(ray.direction, plane.normal);

		if (!Equal(demon, 0.0f, Intersection_ERROR_TOLLERANCE))
		{
			Vec3f pr = plane.center - ray.origin;
			real32 nume = Dot(pr, plane.normal);

			real32 t = nume / demon;

			info->t = t;
			info->normal = plane.normal;
			info->closePoint = TravelDownRay(ray, t);

			result = t > 0.0f;
		}

		return result;
	}

	bool RaycastSphere(const Ray& ray, const Sphere& sphere, RaycastInfo* info)
	{
		real32 sphere_rad = GetSphereRadius(sphere);
		Vec3f sphere_center = GetSphereCenter(sphere);
		Vec3f rs = ray.origin - sphere_center;

		real32 a = Dot(ray.direction, ray.direction);
		real32 b = 2.0f * Dot(rs, ray.direction);
		real32 c = Dot(rs, rs) - sphere_rad * sphere_rad;
		real32 d = b * b - 4.0f * a * c;

		bool result = (d > 0.00001f);

		real32 t = -1.0f;

		if (result)
		{
			t = (-b - Sqrt(d)) / (2.0f * a);
		}

		result = (t > 0.00001f);

		Vec3f point = TravelDownRay(ray, t);

		Vec3f n = Normalize(point - sphere_center);

		info->closePoint = point;
		info->farPoint = point - 2.0f * n;
		info->t = t;
		info->normal = n;

		return result;
	}

	bool RaycastAABB(const Ray& ray, const AABB& aabb, RaycastInfo* info)
	{
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

				info->closePoint = TravelDownRay(ray, tmin);
				info->farPoint = TravelDownRay(ray, tmax);
				info->t = tmin;

				Vec3f dmin = info->closePoint - aabb.min;
				Vec3f dmax = info->closePoint - aabb.max;

				Vec3f n1((real32)Equal(dmin.x, 0.0f), (real32)Equal(dmin.y, 0.0f), (real32)Equal(dmin.z, 0.0f));
				Vec3f n2((real32)Equal(dmax.x, 0.0f), (real32)Equal(dmax.y, 0.0f), (real32)Equal(dmax.z, 0.0f));

				info->normal = n2 - n1;

				result = true;
			}
		}

		return result;
	}

	bool RaycastMeshCollider(const Ray& ray, const Array<Triangle>& triangles, RaycastInfo* info)
	{
		bool result = false;

		int32 closeIndex = -1;
		real32 min = REAL_MAX;
		real32 max = REAL_MIN;
		for (int32 i = 0; i < (int32)triangles.count; i++)
		{
			const Triangle& tri = triangles[i];

			real32 t = FLT_MAX;
			result = result || RaycastTriangle(ray, tri, &t);

			if (t < min)
			{
				min = t;
				closeIndex = i;
			}
			if (t > max)
			{
				max = t;
			}
		}

		if (result)
		{
			info->t = min;
			info->normal = triangles[closeIndex].CacluateNormal();
			info->closePoint = TravelDownRay(ray, min);
			info->farPoint = TravelDownRay(ray, max);
		}

		return result;
	}

	bool CheckManifoldSphere(const Sphere& a, const Sphere& b, Manifold* info)
	{
		real32 t = (a.data.w + b.data.w) * (a.data.w + b.data.w);

		Vec3f ab = Vec3f(a.data.x - b.data.x,
			a.data.y - b.data.y,
			a.data.z - b.data.z);

		real32 dist2 = MagSqrd(ab);
		bool result = dist2 <= t;
		if (result)
		{
			info->normal = Normalize(ab);

			Vec3f cA = GetSphereCenter(a);
			Vec3f cB = GetSphereCenter(b);

			real32 rA = GetSphereRadius(a);
			real32 rB = GetSphereRadius(b);

			info->pt_onA_World = cA - info->normal * rA;
			info->pt_onB_World = cB + info->normal * rB;
		}

		return result;
	}

	bool CheckManifoldSphereOBB(const Sphere& sphere, const OBB& obb, Manifold* info)
	{
		*info = {};

		Vec3f sc = GetSphereCenter(sphere);
		real32 sr = GetSphereRadius(sphere);

		Vec3f pointOnOBB = ClosestPointOnOBB(obb, sc);

		Vec3f c = pointOnOBB - sc;
		real32 d1 = Mag(c);

		if (sr > d1)
		{
			// @NOTE: Normalize
			Vec3f n = (c / d1);
			Vec3f pointOnSphere = n * sr;

			info->pt_onA_World = pointOnSphere;
			info->pt_onB_Body = pointOnOBB;
			info->seperationDistance = sr - d1;
			info->normal = -1.0f * n;

			return true;
		}

		return false;
	}

	bool CheckManifoldAABB(const AABB& a, const AABB& b, Manifold* info)
	{
		*info = {};

		Vec3f a_center = GetAABBCenter(a);
		Vec3f b_center = GetAABBCenter(b);

		Vec3f a_radius = GetAABBRadius(a);
		Vec3f b_radius = GetAABBRadius(b);

		Vec3f n = b_center - a_center;

		real32 x_overlap = a_radius.x + b_radius.x - Abs(n.x);
		real32 y_overlap = a_radius.y + b_radius.y - Abs(n.y);
		real32 z_overlap = a_radius.z + b_radius.z - Abs(n.z);

		bool result = false;
		if (x_overlap > 0 && y_overlap > 0 && z_overlap > 0)
		{
			result = true;
			if (x_overlap < y_overlap)
			{
				if (z_overlap < x_overlap)
				{
					info->normal = (n.z < 0) ? Vec3f(0, 0, 1) : Vec3f(0, 0, -1);
					info->seperationDistance = z_overlap;
				}
				else
				{
					info->normal = (n.x < 0) ? Vec3f(1, 0, 0) : Vec3f(-1, 0, 0);
					info->seperationDistance = x_overlap;
				}
			}
			else
			{
				if (z_overlap < y_overlap)
				{
					info->normal = (n.z < 0) ? Vec3f(0, 0, 1) : Vec3f(0, 0, -1);
					info->seperationDistance = z_overlap;
				}
				else
				{
					info->normal = (n.y < 0) ? Vec3f(0, 1, 0) : Vec3f(0, -1, 0);
					info->seperationDistance = y_overlap;
				}
			}
		}

		return result;
	}

	static bool RaySphere(const Vec3f& rayStart, const Vec3f& rayDir, const Vec3f& sphereCenter, real32 r, real32* t1, real32* t2)
	{
		Vec3f m = sphereCenter - rayStart;
		real32 a = Dot(rayDir, rayDir);
		real32 b = Dot(m, rayDir);
		real32 c = Dot(m, m) - r * r;

		real32 delta = b * b - a * c;
		real32 invA = 1.0f / a;

		if (delta < 0)
		{
			return false;
		}

		real32 deltaRoot = Sqrt(delta);
		*t1 = invA * (b - deltaRoot);
		*t2 = invA * (b + deltaRoot);

		return true;
	}

	bool SweepManifoldSphere(const Sphere& sA, const Vec3f& velA, const Sphere& sB, const Vec3f& velB, real32 dt, Manifold* info)
	{
		Vec3f posA = GetSphereCenter(sA);
		Vec3f posB = GetSphereCenter(sB);

		real32 rA = GetSphereRadius(sA);
		real32 rB = GetSphereRadius(sB);

		Vec3f relVelo = velA - velB;
		Vec3f startPtA = posA;
		Vec3f endPtA = startPtA + relVelo * dt;
		Vec3f rayDir = endPtA - startPtA;

		real32 t0 = 0.0f;
		real32 t1 = 0.0f;

		if (MagSqrd(rayDir) < 0.0001f)
		{
			Vec3f ab = posB - posA;
			real32 r = rA + rB + 0.0001f;
			if (MagSqrd(ab) > r * r)
			{
				return false;
			}
		}
		else if (!RaySphere(posA, rayDir, posB, rA + rB, &t0, &t1))
		{
			return false;
		}

		t0 *= dt;
		t1 *= dt;

		if (t1 < 0.0f)
		{
			return false;
		}

		info->toi = (t0 < 0.0f) ? 0.0f : t0;
		if (info->toi > dt)
		{
			return false;
		}

		Vec3f newPosA = posA + velA * info->toi;
		Vec3f newPosB = posB + velB * info->toi;

		Vec3f ab = Normalize(newPosB - newPosA);

		info->pt_onA_World = newPosA + ab * rA;
		info->pt_onB_World = newPosB - ab * rB;

		return true;
	}


}