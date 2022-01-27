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

	bool8 GameMemory::Initialize(uint64 permanentStorageSize, uint64 transientStorageSize, uint64 dynamicStorageSize)
	{
		void* permanentStorageData = (uint8*)malloc(permanentStorageSize);
		void* transientStorageData = (uint8*)malloc(transientStorageSize);
		void* dynamicStorageData = (uint8*)malloc(dynamicStorageSize);

		Assert(permanentStorageData && transientStorageData && dynamicStorageData, "Could not allocate memory !!");

		if (permanentStorageData && transientStorageData)
		{
			GameMemory* memory = new GameMemory(
				permanentStorageData, permanentStorageSize,
				transientStorageData, transientStorageSize,
				dynamicStorageData, dynamicStorageSize);

			SOLINFO("Memory initialized");
			return true;
		}

		return false;
	}

	void GameMemory::Shutdown()
	{
		if (instance->permanentStorage.base) { free(instance->permanentStorage.base); }
		if (instance->transientStorage.base) { free(instance->transientStorage.base); }
		if (instance->dynamicStorage.base) { free(instance->dynamicStorage.base); }

		delete instance;

		instance = nullptr;
	}

	void GameMemory::Copy(void* dst, void* src, uint64 size)
	{
		memcpy(dst, src, size);
	}

	GameMemory::GameMemory(void* permanentStorageData, uint64 permanentStorageSize,
		void* transientStorageData, uint64 transientStorageSize,
		void* dynamicStorageData, uint64 dynamicStorageSize)
	{
		permanentStorage.base = (uint8*)permanentStorageData;
		permanentStorage.size = permanentStorageSize;

		transientStorage.base = (uint8*)transientStorageData;
		transientStorage.size = transientStorageSize;

		dynamicStorage.base = (uint8*)dynamicStorageData;
		dynamicStorage.size = dynamicStorageSize;

		if (permanentStorage.base) { memset(permanentStorage.base, 0, permanentStorageSize); }
		if (transientStorage.base) { memset(transientStorage.base, 0, transientStorageSize); }
		if (dynamicStorage.base) { memset(dynamicStorage.base, 0, dynamicStorageSize); }

		permanentStorage.used = 0;
		transientStorage.used = 0;
		dynamicStorage.used = 0;

		freeHead.size = dynamicStorageSize;
		freeHead.offset = 0;
		freeHead.next = nullptr;

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

	void* GameMemory::DynamicPushSize(uint64 size)
	{
		uint64 pushOffset = 0;

		MemoryNode* currentNode = &freeHead;
		MemoryNode* previousNode = nullptr;

		uint64 totalSize = size + sizeof(MemoryNode);

		while (currentNode)
		{
			if (currentNode->size == totalSize)
			{

			}
			else if (currentNode->size > totalSize)
			{
				pushOffset = currentNode->offset;
				currentNode->size -= totalSize;
				currentNode->offset += totalSize;
				break;
			}

			previousNode = currentNode;
			Assert(currentNode->next, "DynamicPushSize next node was null");
			currentNode = currentNode->next;
		}


		MemoryNode* fullNode = &fullHead;
		while (fullNode->next) { fullNode = fullNode->next; }

		//fullNode->next =

		uint8* offset = &dynamicStorage.base[pushOffset];


		MemoryNode newNode = {};
		newNode.offset = pushOffset;
		newNode.size = totalSize;
		newNode.next = nullptr;



		return nullptr;
	}

}