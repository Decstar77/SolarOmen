#pragma once
#include "Defines.h"
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

	template<typename T>
	class Array
	{
		friend class ProgramMemory;
	public:

		inline uint32 GetCapcity() const { return capcity; }

		inline T& operator[](const uint32& index)
		{
			Assert(index >= 0 && index < capcity, "Array, invalid index");
			return data[index];
		}

		inline T operator[](const uint32& index) const
		{
			Assert(index >= 0 && index < capcity, "Array, invalid index");

			return data[index];
		}

	private:
		Array(T* data, uint32 capcity)
			: data(data), capcity(capcity)
		{}

		T* data;
		uint32 capcity;
	};

	template<typename T>
	class ManagedArray
	{
		friend class GameMemory;
	public:

		inline uint32 GetCount() const { return count; }
		inline uint32 GetCapcity() const { return capcity; }

		inline T* Add(const T& value)
		{
			uint32 index = count; count++;
			Assert(index >= 0 && index < capcity, "Array, to many items");

			data[index] = value;

			return &data[index];
		}

		inline void Remove(const uint32& index)
		{
			Assert(index >= 0 && index < count, "Array invalid remove index ");
			for (uint32 i = index; i < count - 1; i++)
			{
				data[i] = data[i + 1];
			}
			count--;
		}

		inline void Clear()
		{
			count = 0;
		}

		inline T& operator[](const uint32& index)
		{
			Assert(index >= 0 && index < count, "Array, invalid index");
			return data[index];
		}

		inline T operator[](const uint32& index) const
		{
			Assert(index >= 0 && index < count, "Array, invalid index");

			return data[index];
		}

		ManagedArray()
		{
			data = nullptr;
			count = 0;
			capcity = 0;
		}

	private:
		ManagedArray(T* data, uint32 capcity)
			: data(data), capcity(capcity)
		{}

		inline void Release()
		{
			id = 0;
			count = 0;
			capcity = 0;
			data = nullptr;
		}

		T* data;
		uint32 id;
		uint32 count;
		uint32 capcity;
	};

	struct MemoryContainer
	{
		static constexpr uint32 TOTAL_BYTES = Megabytes(4);
		char data[TOTAL_BYTES];

		inline void ZeroOut()
		{
			ZeroArray(data);
		}
	};

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
		inline static Array<T> GetArray()
		{
			MemoryContainer* mem = instance->GetAvailableMemoryContainer();

			Array<T> result(reinterpret_cast<T*>(mem->data),
				MemoryContainer::TOTAL_BYTES / (uint32)sizeof(T));

			return result;
		}

		template<typename T>
		inline static ManagedArray<T> GetManagedArray()
		{
			uint32 id;
			MemoryContainer* mem = instance->GetAvailableMemoryContainer(&id);

			ManagedArray<T> result(reinterpret_cast<T*>(mem->data),
				MemoryContainer::TOTAL_BYTES / (uint32)sizeof(T));

			result.id = id;

			return result;
		}

		template<typename T>
		inline static void Release(T* t)
		{
			instance->ReleaseMemoryContainer(t->id);
			t->Release();
		}

		inline static void ReleaseAllTransientMemory()
		{
			instance->transientStorage.used = 0;
		}

		inline static void AllocateGameMemory(uint64 permanentStorageSize, uint64 transientStorageSize)
		{
			// @NOTE: Using new here to call the constructor, very rare.
			GameMemory* gameMemory = new GameMemory(permanentStorageSize, transientStorageSize);

			Assert(gameMemory, "Could not allocate game memory");
			if (gameMemory)
			{
				instance = gameMemory;
			}
		}
	private:

		inline static GameMemory* instance = nullptr;

		std::mutex lock;

		volatile uint32 memoryContainersCount = 0;
		MemoryContainer memoryContainers[TOTAL_MEMORY_CONTAINERS];

		volatile uint32 memoryContainerFreeListCount = 0;
		uint32 memoryContainersFreeList[TOTAL_MEMORY_CONTAINERS];

		MemoryArena permanentStorage;
		MemoryArena transientStorage;

		inline MemoryContainer* GetAvailableMemoryContainer(uint32* id)
		{
			lock.lock();

			memoryContainerFreeListCount--;
			memoryContainersCount++;

			Assert(memoryContainersCount < TOTAL_MEMORY_CONTAINERS, "Ran out of memory containers");

			uint32 index = memoryContainersFreeList[memoryContainerFreeListCount];
			*id = index;
			MemoryContainer* mem = &memoryContainers[index];
			mem->ZeroOut();

			lock.unlock();

			return mem;
		}

		inline void ReleaseMemoryContainer(uint32 id)
		{
			//lock.lock();

			uint32 index = memoryContainerFreeListCount;
			memoryContainersFreeList[index] = id;

			memoryContainerFreeListCount++;
			memoryContainersCount--;

			//lock.unlock();
		}

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

		GameMemory(uint64 permanentStorageSize, uint64 transientStorageSize)
		{
			permanentStorage.size = permanentStorageSize;
			transientStorage.size = transientStorageSize;
			permanentStorage.base = (uint8*)malloc(permanentStorageSize);
			transientStorage.base = (uint8*)malloc(transientStorageSize);

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

			for (uint32 i = 0; i < TOTAL_MEMORY_CONTAINERS; i++)
			{
				memoryContainers[i].ZeroOut();
				memoryContainersFreeList[i] = 0;
			}

			for (uint32 i = TOTAL_MEMORY_CONTAINERS; i > 0; i--)
			{
				memoryContainersFreeList[i - 1] = TOTAL_MEMORY_CONTAINERS - i;
				memoryContainerFreeListCount++;
			}

			permanentStorage.used = 0;
			transientStorage.used = 0;
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

