//===============================================================//
/*
	____                        _
  / ____|                      (_)
 | |      ___   ___  _ __ ___   _   ___
 | |     / _ \ / __|| '_ ` _ \ | | / __|
 | |____| (_) |\__ \| | | | | || || (__
  \_____|\___/ |___/|_| |_| |_||_| \___|

*/
//===============================================================//

#pragma once
#include "Defines.h"
#include "SolarMemory.h"
#include "SolarArray.h"
namespace cm
{
	//template<typename T>
	//class Array
	//{
	//	inline T& operator[](const uint32& index)
	//	{
	//		Assert(index >= 0 && index < GetLength(), "Array, invalid index");

	//		return data[index];
	//	}

	//	inline T operator[](const uint32& index) const
	//	{
	//		Assert(index >= 0 && index < GetLength(), "Array, invalid index");

	//		return data[index];
	//	}

	//private:
	//	uint64 capcity;
	//	T* data;
	//};


	class CString
	{
	private:
		// @TODO @BUG @CHECK: If the buffer is full there will be no terminating character at the end of the string!!
		// @NOTE: The length of the string does not include the null char
		// @NOTE: Last char is treated as an unsigned int to store the length
		inline static const int32 MAX_NUMBER_SIZE = 22;
		inline static const int32 TOTAL_SIZE = 256;
		inline static const int32 CAPCITY = TOTAL_SIZE - 2;

	public:

		inline int32 GetLength() const
		{
			uint8 temp = (uint8)data[CAPCITY + 1];
			return (int32)temp;
		}

		inline const char* GetCStr() const
		{
			return data;
		}

		inline void Clear()
		{
			for (int32 i = 0; i < CAPCITY + 1; i++)
			{
				data[i] = '\0';
			}

			SetLength(0);
		}

		inline CString& Add(const char& c)
		{
			int32 index = GetLength();
			index++;
			Assert(index < CAPCITY, "String, to many characters");

			if (index < CAPCITY)
			{
				SetLength(index);
				data[index - 1] = c;
			}

			return *this;
		}

		inline CString& Add(const char* c)
		{
			int32 index = GetLength();
			Assert(index + 1 < CAPCITY, "String, to many characters");

			for (int32 i = 0; index < CAPCITY && c[i] != '\0'; i++, index++)
			{
				Assert(index < CAPCITY, "String, to many characters");

				if (index < CAPCITY)
				{
					data[index] = c[i];
					SetLength(index + 1);
				}
			}

			return *this;
		}

		inline CString& Add(const CString& c)
		{
			return Add(c.data);
		}

		inline CString& Add(const int32& v)
		{
			char buf[MAX_NUMBER_SIZE] = {};
			snprintf(buf, MAX_NUMBER_SIZE, "%ld", v);
			return Add(buf);
		}

		inline CString& Add(const uint32& v)
		{
			char buf[MAX_NUMBER_SIZE] = {};
			snprintf(buf, MAX_NUMBER_SIZE, "%lu", v);
			return Add(buf);
		}

		inline CString& Add(const int64& v)
		{
			char buf[MAX_NUMBER_SIZE] = {};
			snprintf(buf, MAX_NUMBER_SIZE, "%lld", v);
			return Add(buf);
		}

		inline CString& Add(const uint64& v)
		{
			char buf[MAX_NUMBER_SIZE] = {};
			snprintf(buf, MAX_NUMBER_SIZE, "%llu", v);
			return Add(buf);
		}

		inline CString& Add(const real32& v)
		{
			char buf[MAX_NUMBER_SIZE] = {};
			snprintf(buf, MAX_NUMBER_SIZE, "%f", v);
			return Add(buf);
		}

		inline void Replace(const char& c, const char& replaceWith)
		{
			const int32 l = GetLength();
			for (int32 i = 0; i < l; i++)
			{
				if (data[i] == c)
				{
					data[i] = replaceWith;
				}
			}
		}

		inline void RemoveCharacter(const int32& removeIndex)
		{
			const int32 l = GetLength();
			Assert(removeIndex >= 0 && removeIndex < l, "String, invalid index");

			for (int32 i = removeIndex; i < l; i++)
			{
				data[i] = data[i + 1];
			}

			SetLength(l - 1);
		}

		inline void RemoveWhiteSpace()
		{
			for (int32 i = 0; i < GetLength(); i++)
			{
				char d = data[i];
				if (d == ' ' || d == '\n' || d == '\t' || d == '\r')
				{
					RemoveCharacter(i);
					i--;
				}
			}
		}

		inline bool32 Contains(const CString& str)
		{
			int32 otherLength = str.GetLength();
			int32 ourLength = GetLength();
			bool32 result = false;

			for (int32 i = 0; i < ourLength && !result; i++)
			{
				result = true;
				for (int32 j = 0; j < otherLength; j++)
				{
					if (data[i] != str.data[j])
					{
						result = false;
						break;
					}
					else
					{
						i++;
					}
				}
			}

			return result;
		}

		inline bool32 StartsWith(const CString& str)
		{
			const int32 l = GetLength();
			const int32 ll = str.GetLength();
			if (l < ll)
			{
				return false;
			}

			for (int32 i = 0; i < ll; i++)
			{
				if (data[i] != str.data[i])
				{
					return false;
				}
			}

			return true;
		}

		inline void CopyFrom(const CString& src, const int32& start, const int32& end)
		{
			Assert(start >= 0 && start < src.GetLength(), "String, invalid index");
			Assert(end >= 0 && end < src.GetLength(), "String, invalid index");

			int32 writePtr = 0;
			for (int32 i = start; i <= end; i++, writePtr++)
			{
				data[writePtr] = src.data[i];
			}

			SetLength(writePtr);
		}

		// @NOTE: This array is transietory !!!
		Array<CString> Split(const char& delim) const
		{
			Array<CString> result = GameMemory::PushTransientArray<CString>(10);

			const int32 len = GetLength();

			int32 start = 0;
			int32 end = 0;
			for (; end < len; end++)
			{
				if (data[end] == delim)
				{
					if (start != end)
					{
						result[result.count].CopyFrom(*this, start, end - 1);
						result.count++;
						start = end + 1;
					}
					else
					{
						start++;
					}
				}
			}

			if (end != start)
			{
				result[result.count].CopyFrom(*this, start, end - 1);
				result.count++;
			}

			return result;
		}

		inline void ToUpperCase()
		{
			int32 l = GetLength();
			for (int32 i = 0; i < l; i++)
			{
				data[i] = (char)toupper(data[i]);
			}
		}

		inline int32 ToInt32() const
		{
			int32 result = atol(data);

			return result;
		}

		inline int64 ToUint64() const
		{
			uint64 result = atoll(data);

			return result;
		}

		inline real32 ToReal32() const
		{
			real32 result = (real32)atof(data);

			return result;
		}

		CString()
		{
			Clear();
			SetLength(0);
		}

		explicit CString(const char& c)
		{
			Clear();
			data[0] = c;
			SetLength(1);
		}

		CString(const char* str)
		{
			const int32 stringLength = (int32)strlen(str);
			Assert(stringLength < CAPCITY, "String is too large");

			SetLength(stringLength);

			for (int32 i = 0; i < stringLength; i++)
			{
				data[i] = str[i];
			}

			for (int32 i = stringLength; i < CAPCITY; i++)
			{
				data[i] = '\0';
			}
		}

		inline char& operator[](const int32& index)
		{
			Assert(index >= 0 && index < GetLength(), "String, invalid index");

			return data[index];
		}

		inline char operator[](const int32& index) const
		{
			Assert(index >= 0 && index < GetLength(), "String, invalid index");

			return data[index];
		}

		inline bool operator==(const CString& other) const
		{
			int32 index = 0;
			const int32 l = GetLength();
			const int32 o = other.GetLength();
			while (index < l)
			{
				if (index >= o || data[index] != other.data[index])
				{
					return false;
				}
				index++;
			}

			return true;
		}

		inline bool operator!=(const CString& other) const
		{
			return !(*this == other);
		}

		inline bool operator==(const char* other)
		{
			int32 index = 0;
			const int32 l = GetLength();
			const int32 o = (int32)strlen(other);
			while (index < l)
			{
				if (index >= o || data[index] != other[index])
				{
					return false;
				}
				index++;
			}

			return true;
		}

	private:
		char data[TOTAL_SIZE];

		inline void SetLength(const int32 l)
		{
			data[CAPCITY + 1] = (uint8)l;
			//*((uint8*)data[CAPCITY]) =  (uint8)l;
		}
	};

	namespace Util
	{
		inline CString StripFilePath(const CString& str)
		{
			Array<CString> pathElements = str.Split('/');
			CString result = pathElements[pathElements.count - 1];

			return result;
		}

		inline CString StripFileExtension(const CString& str)
		{
			Array<CString> pathElements = str.Split('.');
			CString result = pathElements[0];

			return result;
		}

		inline CString StripFilePathAndExtentions(const CString& str)
		{
			CString result = StripFilePath(str);
			result = StripFileExtension(result);

			return result;
		}

		inline CString GetFileExtension(const CString& str)
		{
			Array<CString> pathElements = str.Split('.');
			return pathElements[pathElements.count - 1];
		}
	};

	template<uint32 size>
	class LargeString
	{
	public:
		CString GetLine()
		{
			uint64 capcity = ArrayCount(data);

			CString result;

			if (data)
			{
				for (; current < capcity; current++)
				{
					if (data[current] == '\n')
					{
						current++;
						break;
					}
					result.Add(data[current]);
				}
			}

			return result;
		}

		LargeString& Add(const CString& str)
		{
			uint64 capcity = ArrayCount(data);
			Assert(str.GetLength() + current < capcity, "Large string has to many characters");

			if (str.GetLength() + current < capcity)
			{
				for (int32 i = 0; i < str.GetLength(); i++)
				{
					this->data[current] = str[i];
					current++;
				}
			}

			return *this;
		}

		inline LargeString& Add(const char& c)
		{
			return Add(CString(c));
		}

		inline LargeString& Add(const real32& c)
		{
			return Add(CString().Add(c));
		}

		inline LargeString& Add(const int32& c)
		{
			return Add(CString().Add(c));
		}

		inline LargeString& Add(const uint32& c)
		{
			return Add(CString().Add(c));
		}

		inline LargeString& Add(const uint64& c)
		{
			return Add(CString().Add(c));
		}

		inline void Clear()
		{
			uint64 capcity = ArrayCount(data);
			for (int32 i = 0; i < capcity; i++)
			{
				data[i] = '\0';
			}

			current = 0;
		}

		inline uint64 GetLength() const
		{
			return current;
		}

		inline char* GetCStr()
		{
			return &data[0];
		}

	private:
		LargeString()
		{
			current = 0;
		}

		uint64 current;
		char data[size];
	};
}



