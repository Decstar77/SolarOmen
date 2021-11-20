#include "SolarSpline.h"


namespace cm
{
	void CatmullRomSpline::AddWaypoint(const SplineWaypoint& waypoint)
	{
		waypoints.AddIfPossible(waypoint);
	}

	void CatmullRomSpline::ComputeDistances()
	{
		totalLength = 0;
		distances.count = waypoints.count;
		for (uint32 i = 0; i < waypoints.count; i++)
		{
			real32 segLength = CalculateSegmentLength(i);
			distances[i] = segLength;
			totalLength += segLength;
		}
	}

	real32 CatmullRomSpline::WrapValue(real32 t)const
	{
		if (t >= (real32)waypoints.count)
		{
			t -= (real32)waypoints.count;
		}

		if (t < 0.0f)
		{
			t += (real32)waypoints.count;
		}

		return t;
	}

	Vec3f CatmullRomSpline::GetSplinePoint(real32 t) const
	{
		Assert(waypoints.count >= 4, "CatmullRomSpline not enough waypoints");

		int32 i1 = (int32)t % waypoints.count;
		int32 i2 = (i1 + 1) % waypoints.count;
		int32 i3 = (i2 + 1) % waypoints.count;
		int32 i0 = i1 >= 1 ? (i1 - 1) : (waypoints.count - 1);

		t = t - (int32)t;

		real32 tt = t * t;
		real32 ttt = tt * t;

		real32 q1 = -ttt + 2.0f * tt - t;
		real32 q2 = 3.0f * ttt - 5.0f * tt + 2.0f;
		real32 q3 = -3.0f * ttt + 4.0f * tt + t;
		real32 q4 = ttt - tt;

		real32 dx = 0.5f * (waypoints[i0].position.x * q1 +
			waypoints[i1].position.x * q2 +
			waypoints[i2].position.x * q3 +
			waypoints[i3].position.x * q4);

		real32 dy = 0.5f * (waypoints[i0].position.y * q1 +
			waypoints[i1].position.y * q2 +
			waypoints[i2].position.y * q3 +
			waypoints[i3].position.y * q4);

		real32 dz = 0.5f * (waypoints[i0].position.z * q1 +
			waypoints[i1].position.z * q2 +
			waypoints[i2].position.z * q3 +
			waypoints[i3].position.z * q4);

		return Vec3f(dx, dy, dz);
	}

	Vec3f CatmullRomSpline::GetSplineGradient(real32 t) const
	{
		int32 i1 = (int32)t % waypoints.count;
		int32 i2 = (i1 + 1) % waypoints.count;
		int32 i3 = (i2 + 1) % waypoints.count;
		int32 i0 = i1 >= 1 ? (i1 - 1) : (waypoints.count - 1);

		t = t - (int32)t;

		real32 tt = t * t;
		real32 ttt = tt * t;

		real32 q1 = -3.0f * tt + 4.0f * t - 1;
		real32 q2 = 9.0f * tt - 10.0f * t;
		real32 q3 = -9.0f * tt + 8.0f * t + 1.0f;
		real32 q4 = 3.0f * tt - 2.0f * t;

		real32 dx = 0.5f * (waypoints[i0].position.x * q1 +
			waypoints[i1].position.x * q2 +
			waypoints[i2].position.x * q3 +
			waypoints[i3].position.x * q4);

		real32 dy = 0.5f * (waypoints[i0].position.y * q1 +
			waypoints[i1].position.y * q2 +
			waypoints[i2].position.y * q3 +
			waypoints[i3].position.y * q4);

		real32 dz = 0.5f * (waypoints[i0].position.z * q1 +
			waypoints[i1].position.z * q2 +
			waypoints[i2].position.z * q3 +
			waypoints[i3].position.z * q4);

		return Vec3f(dx, dy, dz);
	}

	Vec3f CatmullRomSpline::GetSplineNormal(real32 t) const
	{
		int32 i0 = (int32)t % waypoints.count;
		int32 i1 = (i0 + 1) % waypoints.count;

		t = t - (int32)t;

		Vec3f n0 = waypoints[i0].normal;
		Vec3f n1 = waypoints[i1].normal;

		return Normalize(Lerp(n0, n1, t));

		//int32 i1 = (int32)t % waypoints.count;
		//int32 i2 = (i1 + 1) % waypoints.count;
		//int32 i3 = (i2 + 1) % waypoints.count;
		//int32 i0 = i1 >= 1 ? (i1 - 1) : (waypoints.count - 1);
		//
		//t = t - (int32)t;
		//
		//Vec3f n0 = waypoints[i0].normal;
		//Vec3f n1 = waypoints[i1].normal;
		//Vec3f n2 = waypoints[i2].normal;
		//Vec3f n3 = waypoints[i3].normal;
		//
		//Vec3f nn0 = Lerp(n0, n1, t);
		//Vec3f nn1 = Lerp(n2, n3, t);
		//
		//Vec3f nnn = Lerp(nn0, nn1, t);
		//
		//return Normalize(nnn);
	}

	real32 CatmullRomSpline::GetSplineSegmentLength(real32 t) const
	{
		int32 index = (int32)t;
		real32 dist = distances[index];

		return dist;
	}

	real32 CatmullRomSpline::CalculateSegmentLength(int32 index) const
	{
		real32 length = 0.0f;
		real32 stepSize = 0.005f;

		Vec3f oldPoint = GetSplinePoint((real32)index);

		for (real32 t = stepSize; t <= 1.0f; t += stepSize)
		{
			Vec3f newPoint = GetSplinePoint((real32)index + t);
			length += Distance(oldPoint, newPoint);
			oldPoint = newPoint;
		}

		return length;
	}

}