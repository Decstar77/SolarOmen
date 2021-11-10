#pragma once
#include "Defines.h"

namespace cm
{
	template<typename T>
	class Array
	{
	public:
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

		Array(T* data, uint32 capcity);
	};

	template<typename T>
	inline Array<T>::Array(T* data, uint32 capcity)
	{
		this->count = 0;
		this->data = data;
		this->capcity = capcity;
	}
}