#pragma once
#include "../SolarDefines.h"
#include "SolarLogging.h"
#include "SolarString.h"
#include "SolarClock.h"
#include "SolarPrimitives.h"

namespace sol
{
	class ProfilerClock
	{
	public:
		SOL_API  ProfilerClock(const char* name);
		SOL_API  ~ProfilerClock();
	private:
		String functionName;
		Clock clock;
	};

#define PROFILE_FUNCTION() ProfilerClock __PROFILE_CLOCK__(__func__)

	class DebugState
	{
	public:
		static constexpr uint32 DEBUG_RENDER_VERTEX_COUNT = 20000;
		int32 vertexStride;
		int32 vertexCount;
		int32 vertexSizeBytes;
		int32 nextVertexIndex;
		ManagedArray<real32> vertexData;
	};

	class SOL_API Debug
	{
	public:
		static void DrawLine(const Vec3f& a, const Vec3f& b);
		static void DrawPoint(const Vec3f& p, real32 size = 0.5f);
		static void DrawRay(const Ray& ray);
		static void DrawSphere(const Sphere& sphere);
		static void DrawAABB(const AABB& aabb);
		static void DrawOBB(const OBB& obb);
		static void DrawBasis(const Basisf& basis, const Vec3f& pos);

		static DebugState* GetState();
		//static void DrawTriangle(const Triangle& tri);
		//static void DrawTriangleWithNormal(const Triangle& tri);

	private:
		inline static DebugState debug = {};

	private:
		static bool8 Initialize();
		static void Shutdown();

		friend class Application;
	};


}