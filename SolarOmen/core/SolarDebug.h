#pragma once
#include "Defines.h"
#include "SolarString.h"
#include "SimpleColliders.h"

namespace cm
{

#define GetDebugState() DebugState *ds = DebugState::Get();

	struct DebugState
	{
		static constexpr uint32 DEBUG_RENDER_VERTEX_COUNT = 200000;

		int32 vertex_stride;
		int32 vertex_count;
		int32 vertex_size_bytes;
		int32 next_vertex_index;
		real32 vertex_data[DEBUG_RENDER_VERTEX_COUNT * 3]; // @note vertex_count *  vertex_stride

		inline static DebugState* Get() { return debugState; }
		inline static DebugState* debugState = nullptr;

		FixedArray<CString, 1000> logs;
	};

	namespace Debug
	{
		void Initialize();

		void DrawLine(const Vec3f& a, const Vec3f& b);
		void DrawSphere(const Sphere& sphere);
		void DrawAABB(const AABB& aabb);

		void ClearLogs();
		void LogInfo(const CString& msg);

		void ExecuteCommand(const CString& cmd);
	}
}
