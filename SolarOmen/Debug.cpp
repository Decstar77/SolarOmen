#include "Debug.h"
#include <fstream>
#include <sstream>
#include <iostream>
namespace cm
{
	static DebugState* ds = nullptr;

	void InitializeDebugState()
	{
		ds = (DebugState*)malloc(sizeof(DebugState));
		if (ds)
		{
			ZeroStruct(ds);

			ds->vertex_stride = 3;
			ds->vertex_count = DEBUG_RENDER_VERTEX_COUNT;
			ds->vertex_size_bytes = ds->vertex_count * ds->vertex_stride * sizeof(real32);
			ds->next_vertex_index = 0;
		}
		else
		{
			Assert(0, "Debug not alloced");
		}
	}

	DebugState* GetDebugState()
	{
		return ds;
	}

	void DEBUGDrawLine(const Vec3f& a, const Vec3f& b)
	{
		Assert((ds->next_vertex_index - 6) < ds->vertex_count * ds->vertex_stride,
			"We've run out of space for out debug renderer buffer");

		ds->vertex_data[ds->next_vertex_index] = a.x;
		ds->vertex_data[ds->next_vertex_index + 1] = a.y;
		ds->vertex_data[ds->next_vertex_index + 2] = a.z;
		ds->next_vertex_index += ds->vertex_stride;

		ds->vertex_data[ds->next_vertex_index] = b.x;
		ds->vertex_data[ds->next_vertex_index + 1] = b.y;
		ds->vertex_data[ds->next_vertex_index + 2] = b.z;
		ds->next_vertex_index += ds->vertex_stride;
	}

	void DEBUGDrawRay(const Ray& ray, const real32& dist)
	{
		DEBUGDrawLine(ray.origin, ray.origin + ray.direction * dist);
	}

	void DEBUGDrawPoint(const Vec3f& p, const real32& size)
	{
		DEBUGDrawLine(p - Vec3f(size, 0, 0), p + Vec3f(size, 0, 0));
		DEBUGDrawLine(p - Vec3f(0, size, 0), p + Vec3f(0, size, 0));
		DEBUGDrawLine(p - Vec3f(0, 0, size), p + Vec3f(0, 0, size));
	}

	void DEBUGDrawAABB(const AABB& aabb)
	{
		Vec3f min = aabb.min;
		Vec3f max = aabb.max;

		Vec3f v2 = Vec3f(max.x, min.y, min.z);
		Vec3f v3 = Vec3f(max.x, max.y, min.z);
		Vec3f v4 = Vec3f(min.x, max.y, min.z);

		Vec3f v6 = Vec3f(max.x, min.y, max.z);
		Vec3f v7 = Vec3f(min.x, min.y, max.z);
		Vec3f v8 = Vec3f(min.x, max.y, max.z);

		DEBUGDrawLine(min, v2);
		DEBUGDrawLine(min, v4);
		DEBUGDrawLine(min, v7);
		DEBUGDrawLine(max, v6);
		DEBUGDrawLine(max, v8);
		DEBUGDrawLine(max, v3);
		DEBUGDrawLine(v3, v2);
		DEBUGDrawLine(v3, v4);
		DEBUGDrawLine(v2, v6);
		DEBUGDrawLine(v6, v7);
		DEBUGDrawLine(v8, v7);
		DEBUGDrawLine(v8, v4);
	}

	void DEBUGDrawOBB(const OBB& obb)
	{
		Mat4f mat(obb.basis.mat, obb.center);
		Vec3f extents = obb.extents;

		Vec4f v0 = Vec4f(extents, 1) * mat;
		Vec4f v1 = Vec4f(extents * -1.0f, 1) * mat;

		Vec4f v2 = (Vec4f(-extents.x, extents.y, extents.z, 1)) * mat;
		Vec4f v3 = (Vec4f(extents.x, -extents.y, extents.z, 1)) * mat;
		Vec4f v4 = (Vec4f(extents.x, extents.y, -extents.z, 1)) * mat;

		Vec4f v5 = (Vec4f(-extents.x, -extents.y, extents.z, 1)) * mat;
		Vec4f v6 = (Vec4f(extents.x, -extents.y, -extents.z, 1)) * mat;
		Vec4f v7 = (Vec4f(-extents.x, extents.y, -extents.z, 1)) * mat;

		DEBUGDrawLine(Vec3f(v0), Vec3f(v2));
		DEBUGDrawLine(Vec3f(v0), Vec3f(v4));
		DEBUGDrawLine(Vec3f(v0), Vec3f(v3));
		DEBUGDrawLine(Vec3f(v1), Vec3f(v5));
		DEBUGDrawLine(Vec3f(v1), Vec3f(v7));
		DEBUGDrawLine(Vec3f(v1), Vec3f(v6));
		DEBUGDrawLine(Vec3f(v3), Vec3f(v6));
		DEBUGDrawLine(Vec3f(v3), Vec3f(v5));
		DEBUGDrawLine(Vec3f(v2), Vec3f(v5));
		DEBUGDrawLine(Vec3f(v2), Vec3f(v7));
		DEBUGDrawLine(Vec3f(v4), Vec3f(v7));
		DEBUGDrawLine(Vec3f(v4), Vec3f(v6));
	}

	void DEBUGDrawSphere(const Sphere& sphere)
	{
		Vec3f center = GetSphereCenter(sphere);
		real32 radius = GetSphereRadius(sphere);

		real32 angle_inc = 25.5;
		Vec3f last = Vec3f(center.x, center.y, center.z);

		for (real32 angle = 0; angle <= 360; angle += angle_inc)
		{
			Vec3f next;

			next.x = Cos(DegToRad(angle)) * radius;
			next.z = Sin(DegToRad(angle)) * radius;

			next += center;

			DEBUGDrawLine(last, next);
			last = next;
		}
		last = Vec3f(center.x, center.y, center.z);

		for (real32 angle = 0; angle <= 360; angle += angle_inc)
		{
			Vec3f next;

			next.x = Cos(DegToRad(angle)) * radius;
			next.y = Sin(DegToRad(angle)) * radius;

			next += center;

			DEBUGDrawLine(last, next);
			last = next;
		}
		last = Vec3f(center.x, center.y, center.z);

		for (real32 angle = 0; angle <= 360; angle += angle_inc)
		{
			Vec3f next;

			next.z = Cos(DegToRad(angle)) * radius;
			next.y = Sin(DegToRad(angle)) * radius;

			next += center;

			DEBUGDrawLine(last, next);
			last = next;
		}
	}

	void DEBUGDrawCapsule(const Capsule& capsule)
	{
		Vec3f bot = capsule.bot;
		Vec3f top = capsule.top;

		Vec3f dir = Normalize(capsule.top - capsule.bot);

		real32 angle_inc = 25.5;
		real32 radius = capsule.radius;

		Sphere s1 = CreateSphere(bot, radius);
		Sphere s2 = CreateSphere(top, radius);

		DEBUGDrawSphere(s1);
		DEBUGDrawSphere(s2);

		Vec3f up = Vec3f(0, 1, 0);
		Vec3f forward = Normalize(dir);
		Vec3f right = Normalize(Cross(up, forward));

		if (Equal(right, Vec3f(0.0f)))
		{
			up = Vec3f(-1, 0, 0);
			right = Normalize(Cross(up, forward));
		}

		up = (Cross(forward, right));

		DEBUGDrawLine(bot + right * radius, top + right * radius);
		DEBUGDrawLine(bot - right * radius, top - right * radius);
		DEBUGDrawLine(bot + up * radius, top + up * radius);
		DEBUGDrawLine(bot - up * radius, top - up * radius);
	}

	void DEBUGDrawTriangle(const Triangle& tri)
	{
		DEBUGDrawLine(tri.v0, tri.v1);
		DEBUGDrawLine(tri.v1, tri.v2);
		DEBUGDrawLine(tri.v2, tri.v0);
	}

	void DEBUGDrawTriangleWithNormal(const Triangle& tri)
	{
		DEBUGDrawLine(tri.v0, tri.v1);
		DEBUGDrawLine(tri.v1, tri.v2);
		DEBUGDrawLine(tri.v2, tri.v0);

		Vec3f n = Normalize(Cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
		Vec3f p = (tri.v0 + tri.v1 + tri.v2) * 1.0f / 3.0f;
		DEBUGDrawLine(p, p + n);
	}

	void DEBUGDrawFrustum(const Frustrum& frustum)
	{
		FrustrumCorners corners = CalculateFrustrumCorners(frustum);

		DEBUGDrawLine(corners.far_top_left, corners.far_top_right);
		DEBUGDrawLine(corners.far_top_right, corners.far_bottom_right);
		DEBUGDrawLine(corners.far_bottom_right, corners.far_bottom_left);
		DEBUGDrawLine(corners.far_bottom_left, corners.far_top_left);
		DEBUGDrawLine(corners.near_top_left, corners.near_top_right);
		DEBUGDrawLine(corners.near_top_right, corners.near_bottom_right);
		DEBUGDrawLine(corners.near_bottom_right, corners.near_bottom_left);
		DEBUGDrawLine(corners.near_bottom_left, corners.near_top_left);

		DEBUGDrawLine(corners.far_top_left, corners.near_top_left);
		DEBUGDrawLine(corners.far_top_right, corners.near_top_right);
		DEBUGDrawLine(corners.far_bottom_left, corners.near_bottom_left);
		DEBUGDrawLine(corners.far_bottom_right, corners.near_bottom_right);
	}

	void DEBUGDrawBasis(const Basisf& basis, const Vec3f& pos)
	{
		DEBUGDrawLine(pos, pos + basis.forward);
		DEBUGDrawLine(pos, pos + basis.upward);
		DEBUGDrawLine(pos, pos + basis.right);
	}

	class DebugLogger
	{
		std::ofstream file;

	public:
		DebugLogger()
		{
			file.open("SolarDumpTruck.txt");
		}

		~DebugLogger()
		{
			file.close();
		}

		void Log(const std::string& s)
		{
			std::cout << s << std::endl;
			file << s << "\n";
		}
	};

	static DebugLogger logger;

	void DEBUGLog(const CString& str)
	{
		logger.Log(str.GetCStr());
	}
}