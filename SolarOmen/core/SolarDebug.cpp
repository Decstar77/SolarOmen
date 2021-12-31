#include "SolarDebug.h"
#include "core/SolarEntity.h"
#include "game/TankGame.h"
#include "SolarPlatform.h"
#include <iostream>

namespace cm
{
	namespace Debug
	{
		void Debug::Initialize(int32 argc, const char* argv[])
		{
			DebugState::debugState = (DebugState*)malloc(sizeof(DebugState));
			GetDebugState();
			GetPlatofrmState();
			if (ds)
			{
				ZeroStruct(ds);

				ds->vertex_stride = 3;
				ds->vertex_count = DebugState::DEBUG_RENDER_VERTEX_COUNT;
				ds->vertex_size_bytes = ds->vertex_count * ds->vertex_stride * sizeof(real32);
				ds->next_vertex_index = 0;
			}
			else
			{
				Assert(0, "Debug not alloced");

			}

			if (argc >= 4)
			{
				int32 x = atol(argv[1]);
				int32 y = atol(argv[2]);
				Platform::SetWindowPosition(ps, x, y);

				if (strcmp(argv[3], "host"))
				{
					DebugState::host = true;
				}
				else if (strcmp(argv[3], "peer"))
				{
					DebugState::peer = true;
				}
			}

			LogFile("Start");
		}

		void Shutdown()
		{
			GetDebugState();
			LargeString<100000>* largeString = GameMemory::PushPermanentStruct<LargeString<100000>>();;
			for (uint32 i = 0; i < ds->fileLogs.count; i++)
			{
				largeString->Add(ds->fileLogs[i]).Add("\n");
			}

			Platform::WriteFile("dump.txt", largeString->GetCStr(), (uint32)largeString->GetLength());
		}

		void DrawPoint(const Vec3f& p, real32 size)
		{
			DrawLine(p - Vec3f(size, 0, 0), p + Vec3f(size, 0, 0));
			DrawLine(p - Vec3f(0, size, 0), p + Vec3f(0, size, 0));
			DrawLine(p - Vec3f(0, 0, size), p + Vec3f(0, 0, size));
		}

		void Debug::DrawLine(const Vec3f& a, const Vec3f& b)
		{
			GetDebugState();

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

		void DrawRay(const Ray& ray)
		{
			DrawLine(ray.origin, ray.origin + ray.direction * 100.0f);
		}

		void Debug::DrawSphere(const Sphere& sphere)
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

		void ClearLogs()
		{
			GetDebugState();
			ds->logs.Clear();
		}

		void Debug::LogInfo(const CString& msg)
		{
			std::cout << msg.GetCStr() << std::endl;

			GetDebugState();
			int32 index = ds->logs.count;
			if (index == ds->logs.GetCapcity())
			{
				ds->logs.Clear();
				index = 0;
			}

			ds->logs[index] = msg;
			ds->logs.count++;
		}

		void LogFile(const CString& msg)
		{
			GetDebugState();
			ds->fileLogs.AddIfPossible(msg);
		}

		void ExecuteCommand(const CString& cmd)
		{
			if (cmd.StartsWith("host"))
			{
				Platform::NetworkStart(54000);
			}
			else if (cmd.StartsWith("con"))
			{
				Platform::NetworkStart(54001);
			}
			else if (cmd.StartsWith("ping"))
			{
				GetGameState();

				SnapShot snap = {};
				snap.type = SnapShotType::PING;
				Platform::NetworkSend(&snap, sizeof(snap), "192.168.0.107", 54001);
			}
			else if (cmd.StartsWith("send"))
			{
				auto arr = cmd.Split(' ');
				if (arr.count == 2)
				{
					int32 port = arr[1].ToInt32();

					char data[64] = { "Hello again" };
					Platform::NetworkSend(data, sizeof(data), "192.168.0.107", port);
				}
				else
				{
					Debug::LogInfo("No port");
				}
				//Platform::NetworkSend(nullptr, 0);
			}
			else if (cmd.StartsWith("pos"))
			{
			}
		}
	}
}