#pragma once
#include "../SolarDefines.h"
#include "SolarMath.h"

namespace sol
{
	typedef real32 real;
#define EPSILON 0.0001f

	struct Ray
	{
		Vec3f origin;
		Vec3f direction;
		static SOL_API Ray Create(const Vec3f& origin, const Vec3f& direction);
		static SOL_API  Vec3f TravelDown(const Ray& ray, const real32& distance);
	};

	struct RaycastInfo
	{
		Vec3f closePoint;
		Vec3f farPoint;
		Vec3f normal;
		real32 t;
	};

	struct Plane
	{
		Vec3f center;
		Vec3f normal;

		static SOL_API Plane Create(const Vec3f& center, const Vec3f& normal);
	};

	class Raycast
	{
	public:
		static SOL_API bool8 CheckPlane(const Ray& ray, const Plane& plane, RaycastInfo* info = nullptr);
	};

	class CheckIntersection
	{
	public:

	};

}