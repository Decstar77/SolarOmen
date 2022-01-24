#include "SolarDebug.h"

namespace sol
{
	ProfilerClock::ProfilerClock(const char* name)
	{
		this->functionName = String(name);
		clock.Start();
	}

	ProfilerClock::~ProfilerClock()
	{
		clock.Update();
		SOLINFO(functionName.Add(" took: ").Add((real32)clock.elapsed * 1000.0f).Add(" ms").GetCStr());
	}

	bool8 Debug::Initialize()
	{
		debug.vertexStride = 3;
		debug.vertexCount = DebugState::DEBUG_RENDER_VERTEX_COUNT;
		debug.vertexSizeBytes = debug.vertexCount * debug.vertexStride * sizeof(real32);
		debug.nextVertexIndex = 0;
		debug.vertexData.Allocate(debug.vertexCount * debug.vertexStride, MemoryType::PERMANENT);

		return true;
	}

	void Debug::DrawLine(const Vec3f& a, const Vec3f& b)
	{
		Assert((debug.nextVertexIndex - 6) < debug.vertexCount * debug.vertexStride,
			"We've run out of space for our debug renderer buffer");

		debug.vertexData[debug.nextVertexIndex] = a.x;
		debug.vertexData[debug.nextVertexIndex + 1] = a.y;
		debug.vertexData[debug.nextVertexIndex + 2] = a.z;
		debug.nextVertexIndex += debug.vertexStride;

		debug.vertexData[debug.nextVertexIndex] = b.x;
		debug.vertexData[debug.nextVertexIndex + 1] = b.y;
		debug.vertexData[debug.nextVertexIndex + 2] = b.z;
		debug.nextVertexIndex += debug.vertexStride;
	}

	void Debug::DrawPoint(const Vec3f& p, real32 size)
	{
		DrawLine(p - Vec3f(size, 0, 0), p + Vec3f(size, 0, 0));
		DrawLine(p - Vec3f(0, size, 0), p + Vec3f(0, size, 0));
		DrawLine(p - Vec3f(0, 0, size), p + Vec3f(0, 0, size));
	}

	void Debug::DrawRay(const Ray& ray)
	{
		DrawLine(ray.origin, ray.origin + ray.direction * 100.0f);
	}

	void Debug::DrawSphere(const Sphere& sphere)
	{
		Vec3f center = sphere.origin;
		real32 radius = sphere.radius;

		real32 angle_inc = 25.5;
		Vec3f last = Vec3f(center.x, center.y, center.z);

		for (real32 angle = 0; angle <= 360; angle += angle_inc)
		{
			Vec3f next;

			next.x = Cos(DegToRad(angle)) * radius;
			next.z = Sin(DegToRad(angle)) * radius;

			next += center;

			DrawLine(last, next);
			last = next;
		}
		last = Vec3f(center.x, center.y, center.z);

		for (real32 angle = 0; angle <= 360; angle += angle_inc)
		{
			Vec3f next;

			next.x = Cos(DegToRad(angle)) * radius;
			next.y = Sin(DegToRad(angle)) * radius;

			next += center;

			DrawLine(last, next);
			last = next;
		}
		last = Vec3f(center.x, center.y, center.z);

		for (real32 angle = 0; angle <= 360; angle += angle_inc)
		{
			Vec3f next;

			next.z = Cos(DegToRad(angle)) * radius;
			next.y = Sin(DegToRad(angle)) * radius;

			next += center;

			DrawLine(last, next);
			last = next;
		}
	}

	void Debug::DrawAABB(const AABB& aabb)
	{
		Vec3f min = aabb.min;
		Vec3f max = aabb.max;

		Vec3f v2 = Vec3f(max.x, min.y, min.z);
		Vec3f v3 = Vec3f(max.x, max.y, min.z);
		Vec3f v4 = Vec3f(min.x, max.y, min.z);

		Vec3f v6 = Vec3f(max.x, min.y, max.z);
		Vec3f v7 = Vec3f(min.x, min.y, max.z);
		Vec3f v8 = Vec3f(min.x, max.y, max.z);

		DrawLine(min, v2);
		DrawLine(min, v4);
		DrawLine(min, v7);
		DrawLine(max, v6);
		DrawLine(max, v8);
		DrawLine(max, v3);
		DrawLine(v3, v2);
		DrawLine(v3, v4);
		DrawLine(v2, v6);
		DrawLine(v6, v7);
		DrawLine(v8, v7);
		DrawLine(v8, v4);
	}

	void Debug::DrawOBB(const OBB& obb)
	{
		Mat4f mat(obb.basis, obb.origin);
		Vec3f extents = obb.extents;

		Vec4f v0 = Vec4f(extents, 1) * mat;
		Vec4f v1 = Vec4f(extents * -1.0f, 1) * mat;

		Vec4f v2 = (Vec4f(-extents.x, extents.y, extents.z, 1)) * mat;
		Vec4f v3 = (Vec4f(extents.x, -extents.y, extents.z, 1)) * mat;
		Vec4f v4 = (Vec4f(extents.x, extents.y, -extents.z, 1)) * mat;

		Vec4f v5 = (Vec4f(-extents.x, -extents.y, extents.z, 1)) * mat;
		Vec4f v6 = (Vec4f(extents.x, -extents.y, -extents.z, 1)) * mat;
		Vec4f v7 = (Vec4f(-extents.x, extents.y, -extents.z, 1)) * mat;

		DrawLine(Vec4ToVec3(v0), Vec4ToVec3(v2));
		DrawLine(Vec4ToVec3(v0), Vec4ToVec3(v4));
		DrawLine(Vec4ToVec3(v0), Vec4ToVec3(v3));
		DrawLine(Vec4ToVec3(v1), Vec4ToVec3(v5));
		DrawLine(Vec4ToVec3(v1), Vec4ToVec3(v7));
		DrawLine(Vec4ToVec3(v1), Vec4ToVec3(v6));
		DrawLine(Vec4ToVec3(v3), Vec4ToVec3(v6));
		DrawLine(Vec4ToVec3(v3), Vec4ToVec3(v5));
		DrawLine(Vec4ToVec3(v2), Vec4ToVec3(v5));
		DrawLine(Vec4ToVec3(v2), Vec4ToVec3(v7));
		DrawLine(Vec4ToVec3(v4), Vec4ToVec3(v7));
		DrawLine(Vec4ToVec3(v4), Vec4ToVec3(v6));
	}

	//void DEBUGDrawCapsule(const Capsule& capsule)
	//{
	//	Vec3f bot = capsule.bot;
	//	Vec3f top = capsule.top;

	//	Vec3f dir = Normalize(capsule.top - capsule.bot);

	//	real32 angle_inc = 25.5;
	//	real32 radius = capsule.radius;

	//	Sphere s1 = CreateSphere(bot, radius);
	//	Sphere s2 = CreateSphere(top, radius);

	//	DEBUGDrawSphere(s1);
	//	DEBUGDrawSphere(s2);

	//	Vec3f up = Vec3f(0, 1, 0);
	//	Vec3f forward = Normalize(dir);
	//	Vec3f right = Normalize(Cross(up, forward));

	//	if (Equal(right, Vec3f(0.0f)))
	//	{
	//		up = Vec3f(-1, 0, 0);
	//		right = Normalize(Cross(up, forward));
	//	}

	//	up = (Cross(forward, right));

	//	DEBUGDrawLine(bot + right * radius, top + right * radius);
	//	DEBUGDrawLine(bot - right * radius, top - right * radius);
	//	DEBUGDrawLine(bot + up * radius, top + up * radius);
	//	DEBUGDrawLine(bot - up * radius, top - up * radius);
	//}

	//void DEBUGDrawFrustum(const Frustrum& frustum)
	//{
	//	FrustrumCorners corners = CalculateFrustrumCorners(frustum);

	//	DEBUGDrawLine(corners.far_top_left, corners.far_top_right);
	//	DEBUGDrawLine(corners.far_top_right, corners.far_bottom_right);
	//	DEBUGDrawLine(corners.far_bottom_right, corners.far_bottom_left);
	//	DEBUGDrawLine(corners.far_bottom_left, corners.far_top_left);
	//	DEBUGDrawLine(corners.near_top_left, corners.near_top_right);
	//	DEBUGDrawLine(corners.near_top_right, corners.near_bottom_right);
	//	DEBUGDrawLine(corners.near_bottom_right, corners.near_bottom_left);
	//	DEBUGDrawLine(corners.near_bottom_left, corners.near_top_left);

	//	DEBUGDrawLine(corners.far_top_left, corners.near_top_left);
	//	DEBUGDrawLine(corners.far_top_right, corners.near_top_right);
	//	DEBUGDrawLine(corners.far_bottom_left, corners.near_bottom_left);
	//	DEBUGDrawLine(corners.far_bottom_right, corners.near_bottom_right);
	//}

	void Debug::DrawBasis(const Basisf& basis, const Vec3f& pos)
	{
		DrawLine(pos, pos + basis.forward);
		DrawLine(pos, pos + basis.upward);
		DrawLine(pos, pos + basis.right);
	}

	DebugState* Debug::GetState()
	{
		return &debug;
	}
}