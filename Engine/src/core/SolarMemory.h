#pragma once
#include "../SolarDefines.h"

namespace sol
{
	enum class SOL_API MemoryType
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

	struct MemoryNode
	{
		uint64 offset; // @NOTE: In bytes
		uint64 size;   // @NOTE: In bytes
		MemoryNode* next;
	};

	class SOL_API GameMemory
	{
	public:
		template<typename T>
		inline static T* PushPermanentCount(uint32 count) { return (T*)instance->PermanentPushSize(sizeof(T) * count); }

		template<typename T>
		inline static T* PushTransientCount(uint32 count) { return (T*)instance->TransientPushSize(sizeof(T) * count); }

		template<typename T>
		inline static T* PushPermanentStruct() { return (T*)instance->PermanentPushSize(sizeof(T)); }

		template<typename T>
		inline static T* PushTransientStruct() { return (T*)instance->TransientPushSize(sizeof(T)); }

		template<typename T>
		inline static T* PushPermanentClass() { void* storage = instance->PermanentPushSize(sizeof(T));	return new (storage) T(); }

		template<typename T>
		inline static T* PushTransientClass() { void* storage = instance->TransientPushSize(sizeof(T));	return new (storage) T(); }

		inline static uint64 GetTheAmountOfPermanentMemoryUsed() { return instance->permanentStorage.used; }
		inline static uint64 GetTheTotalAmountOfPermanentMemoryAllocated() { return instance->permanentStorage.size; }

		inline static uint64 GetTheAmountOfTransientMemoryUsed() { return instance->transientStorage.used; }
		inline static uint64 GetTheTotalAmountOfTransientMemoryAllocated() { return instance->transientStorage.size; }

		inline static void ReleaseAllTransientMemory() { instance->transientStorage.used = 0; }

		static bool8 Initialize(uint64 permanentStorageSize, uint64 transientStorageSize, uint64 dynamicStorageSize);
		static void Shutdown();

		template<typename T>
		inline static void ZeroStruct(T* t) { ZeroOut(t, sizeof(T)); }

		static void Copy(void* dst, void* src, uint64 size);

	private:
		GameMemory(void* permanentStorageData, uint64 permanentStorageSize,
			void* transientStorageData, uint64 transientStorageSize,
			void* dynamicStorageData, uint64 dynamicStorageSize);

		inline static GameMemory* instance = nullptr;

		MemoryArena permanentStorage;
		MemoryArena transientStorage;
		MemoryArena dynamicStorage;

		MemoryNode freeHead;
		MemoryNode fullHead;

		void* TransientPushSize(uint64 size);
		void* PermanentPushSize(uint64 size);

		void* DynamicPushSize(uint64 size);
		void* DynamicFree(uint64 size);

		static void ZeroOut(void* dst, uint64 size);
	};
}

