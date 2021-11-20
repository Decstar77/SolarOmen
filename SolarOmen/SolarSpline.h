#pragma once
#include "core/SolarCore.h"

namespace cm
{
	struct SplineWaypoint
	{
		Vec3f position;
		Vec3f normal;
	};

	class CatmullRomSpline
	{
	public:
		static constexpr uint32 MAX_WAYPOINTS = 128;

		FixedArray<SplineWaypoint, MAX_WAYPOINTS> waypoints;
		FixedArray<real32, MAX_WAYPOINTS> distances;
		real32 totalLength;

		void AddWaypoint(const SplineWaypoint& waypoint);
		void ComputeDistances();
		real32 WrapValue(real32 t) const;
		Vec3f GetSplinePoint(real32 t) const;
		Vec3f GetSplineGradient(real32 t) const;
		Vec3f GetSplineNormal(real32 t) const;
		real32 GetSplineSegmentLength(real32 t) const;

	private:
		real32 CalculateSegmentLength(int32 index) const;
	};


	//struct HermiteCurve
	//{
	//	Vec3f p0;
	//	Vec3f v0;
	//	Vec3f p1;
	//	Vec3f v1;

	//	Vec3f Evaulate(real32 t)
	//	{
	//		real32 t2 = t * t;
	//		real32 t3 = t * t * t;

	//		real32 h0 = 1.0f - 3.0f * t2 + 2.0f * t3;
	//		real32 h1 = t - 2.0f * t2 + t3;
	//		real32 h2 = -t2 + t3;
	//		real32 h3 = 3.0f * t2 - 2.0f * t3;

	//		Vec3f result = h0 * p0 + h1 * v0 + h2 * v1 + h3 * p1;

	//		return result;
	//	}
	//};

	//struct BezierCurve
	//{
	//	Vec3f p0;
	//	Vec3f p1;
	//	Vec3f p2;
	//	Vec3f p3;

	//	Vec3f Evaulate(real32 t)
	//	{
	//		real32 t2 = t * t;
	//		real32 t3 = t * t * t;

	//		Vec3f c0 = p0;
	//		Vec3f c1 = -3.0f * p0 + 3.0f * p1;
	//		Vec3f c2 = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
	//		Vec3f c3 = -1.0f * p0 + 3.0f * p1 - 3.0f * p2 + p3;

	//		Vec3f result = c0 + c1 * t + c2 * t2 + c3 * t3;

	//		return result;
	//	}
	//};

	//struct Biarc
	//{
	//	Vec3f center;
	//	Vec3f axis1;
	//	Vec3f axis2;
	//	real32 radius;
	//	real32 angle;
	//	real32 arclen;
	//};

	//static void BiarcComputeArcs(Biarc* arc1, Biarc* arc2,
	//	const Vec3f& p1, const Vec3f& t1, const Vec3f& p2, const Vec3f& t2)
	//{
	//	const float epsilon = 0.0001f;

	//	Vec3f v = p2 - p1;
	//	real32 vdv = Dot(v, v);

	//	// @NOTE: Control points are equal
	//	if (vdv < epsilon)
	//	{
	//		arc1->center = p1;
	//		arc2->radius = 0.0f;
	//		arc1->axis1 = v;
	//		arc1->axis2 = v;
	//		arc1->angle = 0.0f;
	//		arc1->arclen = 0.0f;

	//		arc2->center = p1;
	//		arc2->radius = 0.0f;
	//		arc2->axis1 = v;
	//		arc2->axis2 = v;
	//		arc2->angle = 0.0f;
	//		arc2->arclen = 0.0f;
	//		return;
	//	}

	//	// @NOTE: Compute the denominator for the quadratic formula
	//	// @NOTE: Calculate d values

	//	Vec3f t = t1 + t2;
	//	real32 vdt = Dot(v, t);
	//	real32 t1dt2 = Dot(t1, t2);
	//	real32 demon = 2.0f * (1.0f - t1dt2);

	//	if (demon < epsilon)
	//	{
	//		real32 vdt2 = Dot(v, t2);
	//		// @NOTE: Case 3 where tangents are equal and v is perpendicular to them
	//		if (Abs(vdt2) < epsilon)
	//		{
	//			real32 vMag = Sqrt(vdv);
	//			real32 invVMag = 1.0f / vMag;

	//			Vec3f planeNormal = Cross(v, t2);
	//			Vec3f perpAxis = Cross(planeNormal, v);

	//			real32 r = vMag * 0.25f;
	//			Vec3f centerToP1 = -0.25f * v;

	//			arc1->center = p1 - centerToP1;
	//			arc1->radius = r;
	//			arc1->axis1 = centerToP1;
	//			arc1->axis2 = r * invVMag * perpAxis;
	//			arc1->angle = PI;
	//			arc1->arclen = PI * r;


	//			arc2->center = p2 + centerToP1;
	//			arc2->radius = r;
	//			arc2->axis1 = -1.0f * centerToP1;
	//			arc2->axis2 = -r * invVMag * perpAxis;
	//			arc2->angle = PI;
	//			arc2->arclen = PI * r;

	//			return;
	//		}
	//		// @NOTE: Case 2
	//		else
	//		{

	//		}
	//	}
	//}
}
