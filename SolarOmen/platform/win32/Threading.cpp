#include "Threading.h"
#include "BlockingQueue.h"
#include <thread>

namespace cm
{
	namespace Platform
	{
#define THREAD_COUNT 4
		static BlockingQueue<WorkEntry> workEntries = {};

		static std::atomic<bool> sleeping[THREAD_COUNT];
		static std::thread threads[THREAD_COUNT];
		static std::atomic<bool> running;

		static void ThreadLoop(int32 threadIndex)
		{
			sleeping[threadIndex].store(true);
			while (running.load())
			{
				WorkEntry entry = workEntries.Take();
				sleeping[threadIndex].store(false);
				entry.Callback(entry.data);
				sleeping[threadIndex].store(true);
			}
		}

		void CreateThreads()
		{
			running.store(true);
			for (int32 i = 0; i < THREAD_COUNT; i++)
			{
				threads[i] = std::thread(ThreadLoop, i);
			}
		}

		void AddWorkEntry(WorkEntry entry)
		{
			workEntries.Push(entry);
		}

		bool HasWork()
		{
			return !workEntries.Empty();
		}

		void WaitForAllWorkToComplete()
		{
			// @SPEED: We just spin lock !!
			bool check = true;
			while (check)
			{
				check = HasWork();
				for (int32 i = 0; i < THREAD_COUNT; i++)
				{
					bool s = !sleeping[i].load();
					if (s)
					{
						check = true;
					}
				}
			}
		}

		WORK_CALLBACK(DummyWork)
		{

		}

		void ShutdownThreads()
		{
			running.store(false);
			for (int32 i = 0; i < THREAD_COUNT; i++)
			{
				AddWorkEntry(WorkEntry(DummyWork, 0));
			}

			for (int32 i = 0; i < THREAD_COUNT; i++)
			{
				threads[i].join();
			}
		}
	}
}
