#pragma once
#include "Defines.h"


namespace cm
{
	template<typename T, uint32 capcity>
	class FixedArray
	{
	public:
		inline uint32 GetCapcity() const
		{
			return capcity;
		}

		inline bool IsFull() const
		{
			return capcity == count;
		}

		inline bool IsEmpty() const
		{
			return count == 0;
		}

		inline T* Add(const T& value)
		{
			uint32 index = count; count++;
			Assert(index >= 0 && index < capcity, "Array, add to many items");

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

		inline void Remove(const T* ptr)
		{
			for (uint32 i = 0; i < count; i++)
			{
				if (ptr == &data[i])
				{
					Remove(i);
					return;
				}
			}
		}

		inline void Clear()
		{
			count = 0;
		}

		inline T* Get(const uint32& index)
		{
			Assert(index >= 0 && index < capcity, "Array, invalid index");
			return &data[index];
		}

		inline const T* Get(const uint32& index) const
		{
			Assert(index >= 0 && index < capcity, "Array, invalid index");
			return &data[index];
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
		count = 0;
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

	template<typename T>
	class Queue
	{
	public:

		void Push(const T& t)
		{
			uint32 index = count;
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

	template<uint32 size>
	struct FixedMemoryStream
	{
		uint32 bufferCursor;
		FixedArray<uint8, size> buffer;

		void BeginBufferLoop() { bufferCursor = 0; }
		bool BufferLoopIncomplete() {
			return bufferCursor < buffer.count;
		}

		template<typename T>
		inline T GetNextType()
		{
			static_assert(std::is_enum<T>::value, "Not enum!!");
			return (T)buffer.data[bufferCursor];
		}

		template<typename T>
		inline T* GetNext()
		{
			uint32 index = bufferCursor;
			if (index < buffer.count)
			{
				bufferCursor += sizeof(T);
				return (T*)&buffer.data[index];
			}

			return nullptr;
		}

		template<typename T>
		bool Add(const T& t)
		{
			int32 bytes = sizeof(T);
			if (bytes + buffer.count < buffer.GetCapcity())
			{
				int32 index = buffer.count;
				buffer.count += bytes;
				memcpy(&buffer.data[index], (void*)(&t), bytes);

				return true;
			}

			return false;
		}
	};

}