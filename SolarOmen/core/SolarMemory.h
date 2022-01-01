#pragma once
#include "Defines.h"

namespace cm
{
	enum class MemoryType
	{
		INVALID = 0,
		PERMANENT,
		TRANSIENT
	};

	struct MemoryArena
	{
		uint8* base;
		uint64 size;
		uint64 used;

		inline void Reset()
		{
			used = 0;
		}

		inline void* PushSize_(uint64 size)
		{
			Assert((used + size) <= this->size, "Memory ran out of memory, not big enough");
			void* Result = base + used;
			used += size;

			return Result;
		}
	};

	class GameMemory
	{
	private:
		static constexpr uint32 TOTAL_MEMORY_CONTAINERS = 16;
		static constexpr uint32 LARGE_STRING_CAPCITY = 100000;

	public:

		template<typename T>
		inline static T* PushPermanentCount(uint32 count)
		{
			return (T*)instance->PermanentPushSize(sizeof(T) * count);
		}

		template<typename T>
		inline static T* PushTransientCount(uint32 count)
		{
			return (T*)instance->TransientPushSize(sizeof(T) * count);
		}

		template<typename T>
		inline static T* PushPermanentStruct()
		{
			return (T*)instance->PermanentPushSize(sizeof(T));
		}

		template<typename T>
		inline static T* PushTransientStruct()
		{
			return (T*)instance->TransientPushSize(sizeof(T));
		}

		template<typename T>
		inline static T* PushTransientClass()
		{
			void* storage = instance->TransientPushSize(sizeof(T));
			T* t = new (storage) T();
			return t;
		}

		inline static uint64 GetTheAmountOfTransientMemoryUsed()
		{
			return instance->transientStorage.used;
		}

		inline static uint64 GetTheTotalAmountOfTransientMemoryAllocated()
		{
			return instance->transientStorage.size;
		}

		inline static uint64 GetTheAmountOfPermanentMemoryUsed()
		{
			return instance->permanentStorage.used;
		}

		inline static uint64 GetTheTotalAmountOfPermanentMemoryAllocated()
		{
			return instance->permanentStorage.size;
		}

		inline static void ReleaseAllTransientMemory()
		{
			instance->transientStorage.used = 0;
		}

		GameMemory(void* permanentStorageData, uint64 permanentStorageSize, void* transientStorageData, uint64 transientStorageSize)
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

	private:

		inline static GameMemory* instance = nullptr;

		std::mutex lock;

		MemoryArena permanentStorage;
		MemoryArena transientStorage;

		inline void* TransientPushSize(uint64 size)
		{
			lock.lock();
			void* result = (void*)instance->transientStorage.PushSize_(size);
			lock.unlock();

			ZeroArrayCount(result, size);

			return result;
		}

		inline void* PermanentPushSize(uint64 size)
		{
			lock.lock();
			void* result = (void*)instance->permanentStorage.PushSize_(size);
			lock.unlock();

			ZeroArrayCount(result, size);

			return result;
		}
	};
}

