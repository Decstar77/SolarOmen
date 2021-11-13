#pragma once
#include "core/SolarCore.h"
#include "SimpleColliders.h"

namespace cm
{
	struct DebugState
	{
		int32 vertex_stride;
		int32 vertex_count;
		int32 vertex_size_bytes;
		int32 next_vertex_index;
#define DEBUG_RENDER_VERTEX_COUNT 200000
		real32 vertex_data[DEBUG_RENDER_VERTEX_COUNT * 3]; // @note vertex_count *  vertex_stride
	};

	void InitializeDebugState();

	DebugState* GetDebugState();

	void DEBUGDrawLine(const Vec3f& a, const Vec3f& b);

	void DEBUGDrawRay(const Ray& ray, const real32& dist = 100.f);

	void DEBUGDrawPoint(const Vec3f& p, const real32& size = 0.1);

	void DEBUGDrawAABB(const AABB& aabb);

	void DEBUGDrawOBB(const OBB& obb);

	void DEBUGDrawSphere(const Sphere& sphere);

	void DEBUGDrawCapsule(const Capsule& capsule);

	void DEBUGDrawTriangle(const Triangle& tri);

	void DEBUGDrawTriangleWithNormal(const Triangle& tri);

	void DEBUGDrawFrustum(const Frustrum& frustum);

	void DEBUGLog(const CString& str);

	template <typename T>
	void Log(const T& val)
	{
		CString str = val.ToString();
		DEBUGLog(str);
	}
}

