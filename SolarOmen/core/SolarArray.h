#pragma once
#include "Defines.h"

namespace cm
{
	template<typename T, uint32 capcity>
	class FixedArray
	{
	public:
		inline T* Add(const T& value)
		{
			uint32 index = count; count++;
			Assert(index >= 0 && index < capcity, "Array, to many items");

			data[index] = value;

			return &data[index];
		}

		// @NOTE: Maybe this is a mistep but oh well....
		inline bool32 AddIfPossible(const T& t)
		{
			uint32 index = count;
			if (index < capcity)
			{
				data[index] = t;
				count++;

				return true;
			}

			return false;
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
			Assert(index >= 0 && index < capcity, "Array, invalid index");
			return data[index];
		}

		inline T operator[](const uint32& index) const
		{
			Assert(index >= 0 && index < capcity, "Array, invalid index");

			return data[index];
		}

		T data[capcity];
		uint32 count;
		FixedArray();
	};

	template<typename T, uint32 size>
	inline FixedArray<T, size>::FixedArray()
	{
	}

	template<typename T>
	class ManagedArray
	{
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

		// @NOTE: Maybe this is a mistep but oh well....
		inline bool32 AddIfPossible(const T& t)
		{
			uint32 index = count;
			if (index < capcity)
			{
				data[index] = t;
				count++;

				return true;
			}

			return false;
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
			Assert(index >= 0 && index < capcity, "Array, invalid index");
			return data[index];
		}

		inline T operator[](const uint32& index) const
		{
			Assert(index >= 0 && index < capcity, "Array, invalid index");

			return data[index];
		}

		T* data;
		uint32 count;
		uint32 capcity;

		ManagedArray();
		ManagedArray(T* data, uint32 capcity);
	};

	template<typename T>
	inline ManagedArray<T>::ManagedArray()
	{
		this->count = 0;
		this->data = 0;
		this->capcity = 0;
	}

	template<typename T>
	inline ManagedArray<T>::ManagedArray(T* data, uint32 capcity)
	{
		this->count = 0;
		this->data = data;
		this->capcity = capcity;
	}

}