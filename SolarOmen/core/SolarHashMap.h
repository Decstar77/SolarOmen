#pragma once
#include "Defines.h"
#include "SolarArray.h"
#include "SolarMemory.h"

namespace cm
{
	template<typename T>
	class HashMap
	{
	public:

		void Put(uint64 key, const T& t);
		T* Get(uint64 key);
		ManagedArray<T> GetValueSet() const;

	private:
		struct Entry
		{
			uint64 key;
			bool32 valid;
			T t;
		};

		uint64 Hash(uint64 x);
		uint32 count;
		FixedArray<FixedArray<Entry, 25>, 250> entries;
	};

	template<typename T>
	inline void HashMap<T>::Put(uint64 key, const T& t)
	{
		uint64 hash = Hash(key);
		uint32 index = (uint32)(hash % entries.GetCapcity());

		FixedArray<Entry, 25>& bucket = entries[index];

		for (uint32 i = 0; i < bucket.GetCapcity(); i++)
		{
			Entry& entry = bucket[i];
			if (!entry.valid)
			{
				entry.key = key;
				entry.valid = true;
				entry.t = t;
				count++;
				return;
			}
		}

		Assert(0, "Could not place item in hashmap, it's full");
	}

	template<typename T>
	inline T* HashMap<T>::Get(uint64 key)
	{
		uint64 hash = Hash(key);
		uint32 index = (uint32)(hash % entries.GetCapcity());

		FixedArray<Entry, 25>& bucket = entries[index];

		for (uint32 i = 0; i < bucket.GetCapcity(); i++)
		{
			Entry* entry = bucket.Get(i);
			if (entry->valid && entry->key == key)
			{
				return &entry->t;
			}
		}

		return nullptr;
	}

	template<typename T>
	inline ManagedArray<T> HashMap<T>::GetValueSet() const
	{
		ManagedArray<T> result;
		result.data = GameMemory::PushTransientCount<T>(count);
		result.capcity = count;

		for (uint32 buckedIndex = 0; buckedIndex < entries.GetCapcity(); buckedIndex++)
		{
			const FixedArray<Entry, 25>& bucket = entries[buckedIndex];

			for (uint32 entryIndex = 0; entryIndex < bucket.GetCapcity(); entryIndex++)
			{
				Entry entry = bucket[entryIndex];

				if (entry.valid)
				{
					result.Add(entry.t);
				}
			}
		}

		return result;
	}

	template<typename T>
	inline uint64 cm::HashMap<T>::Hash(uint64 x)
	{
		// @NOTE: Source https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
		x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
		x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
		x = x ^ (x >> 31);
		return x;
	}

}