#pragma once
#include "Defines.h"
#include "SolarArray.h"
namespace cm
{
	//template<typename T, uint32 CAPCITY>
	//class FreeList
	//{


	//private:
	//	T data[CAPCITY];
	//	uint32 freelist[CAPCITY];

	//	//public:
	//	//	int32 count;
	//	//	std::vector<T> data;
	//	//	std::queue<uint32> freelist;

	//	//public:
	//	//	uint32 Add(const T& t)
	//	//	{
	//	//		if (freelist.size() > 0)
	//	//		{
	//	//			uint32 index = freelist.front(); freelist.pop();
	//	//			data.at(index) = t;
	//	//			count++;

	//	//			return index;
	//	//		}
	//	//		else
	//	//		{
	//	//			uint32 index = static_cast<uint32>(data.size());
	//	//			data.emplace_back(t);
	//	//			count++;

	//	//			return index;
	//	//		}
	//	//	}

	//	//	inline T* Get(const uint32& index)
	//	//	{
	//	//		return &data.at(index);
	//	//	}

	//	//	inline T* Remove(const uint32& index)
	//	//	{
	//	//		freelist.push(index);

	//	//		return &data.at(index);
	//	//	}

	//	//public:

	//	//	FreeList()
	//	//		: count(0)
	//	//	{
	//	//	}

	//	//	FreeList(const uint32& size)
	//	//		: count(0)
	//	//	{
	//	//		data.reserve(size);
	//	//	}

	//	//	~FreeList() {}
	//};

	struct MemoryArena
	{
		uint64 size;
		uint8* base;
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

		inline static MemoryArena* GetPermanentMemoryArena()
		{
			return &instance->permanentStorage;
		}

		inline static MemoryArena* GetTransientMemoryArena()
		{
			return &instance->transientStorage;
		}

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

		template<typename T>
		inline static ManagedArray<T> PushPermanentArray(uint32 capcity)
		{
			ManagedArray<T> arr = ManagedArray<T>((T*)instance->PermanentPushSize(sizeof(T) * capcity), capcity);

			return arr;
		}

		template<typename T>
		inline static ManagedArray<T> PushTransientArray(uint32 capcity)
		{
			ManagedArray<T> arr = ManagedArray<T>((T*)instance->TransientPushSize(sizeof(T) * capcity), capcity);

			return arr;
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


	//struct TemporayArena
	//{
	//	MemoryArena* arena;
	//	size_t used;
	//};

	//inline TemporayArena BeginTemporayMemory(MemoryArena* arena)
	//{
	//	TemporayArena temp = {};

	//	temp.arena = arena;
	//	temp.used = arena->used;

	//	arena->tempory_count++;

	//	return temp;
	//}

	//inline void EndTemporaryMemory(TemporayArena temp_arena)
	//{
	//	MemoryArena* arena = temp_arena.arena;
	//	Assert(arena->used >= temp_arena.used, "Incorrect being and end of memory, check ordering");
	//	arena->used = temp_arena.used;
	//	Assert(arena->tempory_count > 0, "Trying to pop memory that isn't there");
	//	arena->tempory_count--;
	//}

	//inline void CheckArena(MemoryArena* arena)
	//{
	//	Assert(arena->tempory_count == 0, "Didn't end some tempory memory");
	//}
}

