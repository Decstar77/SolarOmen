#pragma once
#include "core/SolarCore.h"

namespace cm
{
#define WORK_CALLBACK(name) void name(void *data)

	namespace Platform
	{
		typedef WORK_CALLBACK(WorkCallback);
		struct WorkEntry
		{
			WorkCallback* Callback;
			void* data;

			WorkEntry(WorkCallback* callback, void* data)
				: Callback(callback), data(data)
			{

			}
		};

		void CreateThreads();
		void AddWorkEntry(WorkEntry entry);
		bool HasWork();
		void WaitForAllWorkToComplete();
		void ShutdownThreads();
	}
}