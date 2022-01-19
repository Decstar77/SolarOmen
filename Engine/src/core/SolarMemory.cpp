#include "SolarMemory.h"
#include "SolarLogging.h"
#include <memory>
#include <mutex>

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
#define PushSize(Arena, size) (void *)PushSize_(Arena, size)
#define ZeroStruct(ptr_data) memset(ptr_data, 0, sizeof(*ptr_data))
#define ZeroArray(ptr_data) memset(ptr_data, 0, sizeof(ptr_data));
#define ZeroArrayCount(ptr_data, count) memset(ptr_data, 0, count);

namespace sol
{
	static std::mutex lock;

	bool8 GameMemory::Initialize(uint64 permanentStorageSize, uint64 transientStorageSize)
	{
		void* permanentStorageData = (uint8*)malloc(permanentStorageSize);
		void* transientStorageData = (uint8*)malloc(transientStorageSize);

		Assert(permanentStorageData && transientStorageData, "Could not allocate memory !!");

		if (permanentStorageData && transientStorageData)
		{
			GameMemory* memory = new GameMemory(permanentStorageData, permanentStorageSize,
				transientStorageData, transientStorageSize);

			SOLINFO("Memory initialized");
			return true;
		}

		return false;
	}

	void GameMemory::Shutdown()
	{
		if (instance->permanentStorage.base) { free(instance->permanentStorage.base); }
		if (instance->transientStorage.base) { free(instance->transientStorage.base); }

		delete instance;

		instance = nullptr;
	}

	void GameMemory::Copy(void* dst, void* src, uint64 size)
	{
		memcpy(dst, src, size);
	}

	GameMemory::GameMemory(void* permanentStorageData, uint64 permanentStorageSize, void* transientStorageData, uint64 transientStorageSize)
	{
		permanentStorage.size = permanentStorageSize;
		transientStorage.size = transientStorageSize;
		permanentStorage.base = (uint8*)permanentStorageData;
		transientStorage.base = (uint8*)transientStorageData;

		Assert(permanentStorage.base, "Could alloc permanent storage");
		if (permanentStorage.base)
		{
			memset(permanentStorage.base, 0, permanentStorageSize);
		}

		Assert(transientStorage.base, "Could alloc transient storage");
		if (transientStorage.base)
		{
			memset(transientStorage.base, 0, transientStorageSize);
		}

		permanentStorage.used = 0;
		transientStorage.used = 0;

		instance = this;
	}

	void* GameMemory::TransientPushSize(uint64 size)
	{
		lock.lock();
		void* result = (void*)instance->transientStorage.PushSize_(size);
		lock.unlock();

		ZeroArrayCount(result, size);

		return result;
	}

	void* GameMemory::PermanentPushSize(uint64 size)
	{
		lock.lock();
		void* result = (void*)instance->permanentStorage.PushSize_(size);
		lock.unlock();

		ZeroArrayCount(result, size);

		return result;
	}

	void GameMemory::ZeroOut(void* dst, uint64 size)
	{
		memset(dst, 0, size);
	}
}