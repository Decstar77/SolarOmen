#pragma once
#include "Defines.h"


namespace cm
{
	template<typename T>
	class Queue
	{
	public:

		void Push(const T& t)
		{
			int32 index = count;
			Assert(index < capcity, "Queue, can't push more values");

			data[index] = t;
			count++;
		}

		T Pop()
		{
			int32 index = count - 1;
			Assert(index >= 0, "Queue, can't pop any values");
			count--;

			return data[index];
		}

		inline T& operator[](const uint32& index)
		{
			Assert(index >= 0 && index < capcity, "Queue, invalid index");
			return data[index];
		}

		inline T operator[](const uint32& index) const
		{
			Assert(index >= 0 && index < capcity, "Queue, invalid index");

			return data[index];
		}

		T* data;
		uint32 count;
		uint32 capcity;

		Queue(T* data, uint32 capcity);
	};


	template<typename T>
	inline Queue<T>::Queue(T* data, uint32 capcity)
	{
		this->count = 0;
		this->data = data;
		this->capcity = capcity;
	}

	template<typename T>
	class CircularQueue
	{
	public:
		void Push(const T& t)
		{
			int32 index = end;
			data[index] = t;
			Advance();
		}

		T Pop()
		{
			count--;
			Assert(count >= 0, "Queue is empty");

			end = DecAndWrapValue(end);

			return data[end];
		}

		T Peek()
		{
			return data[DecAndWrapValue(end)];
		}


		T* data;
		int32 start;
		int32 end;
		int32 count;
		int32 capcity;

		CircularQueue(T* data, int32 capcity);

	private:
		inline void Advance()
		{
			end = IncAndWrapValue(end);
			if (end == start)
			{
				start = IncAndWrapValue(start);
			}
			else
			{
				count++;
			}
		}

		inline int32 DecAndWrapValue(int32 v) {
			v--;
			if (v < 0) { v = capcity - 1; }
			return v;
		}

		inline int32 IncAndWrapValue(int32 v) {
			v++;
			if (v > capcity - 1) { v = 0; }
			return v;
		}
	};

	template<typename T>
	inline CircularQueue<T>::CircularQueue(T* data, int32 capcity)
	{
		this->start = 0;
		this->end = 0;
		this->count = 0;
		this->capcity = capcity;
		this->data = data;
	}
}