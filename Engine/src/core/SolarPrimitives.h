#pragma once
#include "SolarMath.h"

namespace sol
{
#define EPSILON 0.0001f

	template<typename T>
	struct ImplicitRaycastInfo
	{
		Vec3<T> point;
		Vec3<T> normal;
		T t;
	};

	typedef ImplicitRaycastInfo<real32> RaycastInfo;
	typedef ImplicitRaycastInfo<real64> PreciseRaycastInfo;

	template<typename T>
	struct ImplicitRay
	{
		Vec3<T> origin;
		Vec3<T> direction;

		ImplicitRay() {};
		ImplicitRay(const Vec3<T>& origin, const Vec3<T>& direction) : origin(origin), direction(direction) {}
	};

	typedef ImplicitRay<real32> Ray;
	typedef ImplicitRay<real64> PreciseRay;

	template<typename T>
	inline Vec3<T> TravelDown(const ImplicitRay<T>& ray, const T& distance)
	{
		return ray.origin + distance * ray.direction;
	}

	template<typename T>
	struct ImplicitPlane
	{
		Vec3<T> origin;
		Vec3<T> normal;

		ImplicitPlane() {};
		ImplicitPlane(const Vec3<T>& origin, const Vec3<T>& normal) : origin(origin), normal(normal) {}
	};

	typedef ImplicitPlane<real32> Plane;
	typedef ImplicitPlane<real64> PrecisePlane;

	template<typename T>
	struct ImplicitSphere
	{
		Vec3<T> origin;
		T radius;

		ImplicitSphere() : radius(0) {};
		ImplicitSphere(const Vec3<T>& origin, T radius) : origin(origin), radius(radius) {};
	};

	typedef ImplicitSphere<real32> Sphere;
	typedef ImplicitSphere<real64> PreciseSphere;

	template<typename T>
	struct ImplicitAABB
	{
		Vec3<T> min;
		Vec3<T> max;

		ImplicitAABB() {};
		ImplicitAABB(const Vec3<T>& min, const Vec3<T>& max) : min(min), max(max) {};
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

	template<typename T>
	struct ImplicitOBB
	{
		Vec3<T> extents;
		Vec3<T> origin;
		Mat3<T> basis;

		ImplicitOBB() {}
		ImplicitOBB(const Vec3<T>& extents, const Vec3<T>& origin)
		{
			this->extents = extents;
			this->origin = origin;
			basis = Mat3<T>(1);
		}
	};

	typedef ImplicitOBB<real32> OBB;
	typedef ImplicitOBB<real64> PreciseOBB;

	class Raycast
	{
	public:
		template<typename T>
		static bool8 CheckPlane(const ImplicitRay<T>& ray, const ImplicitPlane<T>& plane, ImplicitRaycastInfo<T>* info)
		{
			bool result = false;
			real32 demon = Dot(ray.direction, plane.normal);

			if (!Equal(demon, 0.0f, EPSILON))
			{
				Vec3f pr = plane.origin - ray.origin;
				real32 nume = Dot(pr, plane.normal);

				real32 t = nume / demon;

				if (info)
				{
					info->t = t;
					info->normal = plane.normal;
					info->point = TravelDown(ray, t);
				}

				result = t > 0.0f;
			}

			return result;
		}

		//template<typename T>
		//static bool8 CheckSphere(const ImplicitRay<T>& ray, const ImplicitSphere<T>& sphere, ImplicitRaycastInfo<T>* info)
		//{
		//	Vec3<T> oc = ray.origin - sphere.origin;
		//	auto a = MagSqrd(ray.direction);
		//	auto half_b = Dot(oc, ray.direction);
		//	auto c = MagSqrd(oc) - sphere.radius * sphere.radius;

		//	auto discriminant = half_b * half_b - a * c;
		//	if (discriminant < 0) return false;
		//	auto sqrtd = Sqrt(discriminant);

		//	auto root = (-half_b - sqrtd) / a;

		//	info->t = root;
		//	info->point = TravelDown(ray, info->t);
		//	info->normal = (info->p - sphere.origin) / sphere.radius;

		//	return true;
		//}

		// @NOTE: Thanks to ray tracing gems 2. 
		template<typename T>
		static bool8 CheckAABB(const ImplicitRay<T>& ray, const ImplicitAABB<T>& aabb, ImplicitRaycastInfo<T>* info)
		{
			Vec3<T> tmin = (aabb.min - ray.origin) / ray.direction;
			Vec3<T> tmax = (aabb.max - ray.origin) / ray.direction;

			Vec3<T> sc = Min(tmin, tmax);
			Vec3<T> sf = Max(tmin, tmax);

			T t0 = Max(sc.x, sc.y, sc.z);
			T t1 = Min(sf.x, sf.y, sf.z);

			if (t0 <= t1 && t1 > static_cast<T>(0.0))
			{
				if (info)
				{
					info->t = t0;
					info->point = TravelDown(ray, t0);

					Vec3<T> a = Abs(info->point - aabb.min);
					Vec3<T> b = Abs(info->point - aabb.max);

					// @TODO: Make fast
					T min = a.x;
					info->normal = Vec3<T>(-1, 0, 0);
					if (a.y < min) { info->normal = Vec3<T>(0, -1, 0); min = a.y; }
					if (a.z < min) { info->normal = Vec3<T>(0, 0, -1); min = a.z; }
					if (b.x < min) { info->normal = Vec3<T>(1, 0, 0); min = b.x; }
					if (b.y < min) { info->normal = Vec3<T>(0, 1, 0); min = b.y; }
					if (b.z < min) { info->normal = Vec3<T>(0, 0, 1); min = b.z; }
				}

				return true;
			}

			return false;
		}

		// @NOTE: Thanks to ray tracing gems 2. 
		template<typename T>
		static bool8 CheckOBB(const ImplicitRay<T>& ray, const ImplicitOBB<T>& obb, ImplicitRaycastInfo<T>* info)
		{
			Mat4<T> m = Mat4<T>(obb.basis, obb.origin);
			Mat4<T> invm = Inverse(m);

			ImplicitRay<T> rPrime = {};
			rPrime.origin = Vec4ToVec3(Vec4<T>(ray.origin, static_cast<T>(1.0)) * invm);
			rPrime.direction = Vec4ToVec3(Vec4<T>(ray.direction, static_cast<T>(0.0)) * invm);

			ImplicitAABB<T> bPrime = {};
			bPrime.min = static_cast<T>(-1.0) * obb.extents;
			bPrime.max = static_cast<T>(1.0) * obb.extents;

			if (Raycast::CheckAABB(rPrime, bPrime, info))
			{
				if (info)
				{
					info->point = Vec4ToVec3(Vec4<T>(info->point, static_cast<T>(1.0)) * m);
					info->normal = Normalize(Vec4ToVec3(Vec4<T>(info->normal, static_cast<T>(0.0)) * Transpose(invm)));
				}

				return true;
			}

			return false;
		}

	};

	class CheckIntersection
	{
	public:

	};

}