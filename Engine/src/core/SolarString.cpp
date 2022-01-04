
#include "SolarString.h"

#include <string>

namespace sol
{
	inline int32 String::GetLength() const
	{
		uint8 temp = (uint8)data[CAPCITY + 1];
		return (int32)temp;
	}

	inline const char* String::GetCStr() const
	{
		return data;
	}

	inline char* String::GetCStr()
	{
		return data;
	}

	inline void String::Clear()
	{
		for (int32 i = 0; i < CAPCITY + 1; i++)
		{
			data[i] = '\0';
		}

		SetLength(0);
	}

	inline String& String::Add(const char& c)
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

	inline String& String::Add(const char* c)
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

	inline String& String::Add(const String& c)
	{
		return Add(c.data);
	}

	inline String& String::Add(const int32& v)
	{
		char buf[MAX_NUMBER_SIZE] = {};
		snprintf(buf, MAX_NUMBER_SIZE, "%ld", v);
		return Add(buf);
	}

	inline String& String::Add(const uint32& v)
	{
		char buf[MAX_NUMBER_SIZE] = {};
		snprintf(buf, MAX_NUMBER_SIZE, "%lu", v);
		return Add(buf);
	}

	inline String& String::Add(const int64& v)
	{
		char buf[MAX_NUMBER_SIZE] = {};
		snprintf(buf, MAX_NUMBER_SIZE, "%lld", v);
		return Add(buf);
	}

	inline String& String::Add(const uint64& v)
	{
		char buf[MAX_NUMBER_SIZE] = {};
		snprintf(buf, MAX_NUMBER_SIZE, "%llu", v);
		return Add(buf);
	}

	inline String& String::Add(const real32& v)
	{
		char buf[MAX_NUMBER_SIZE] = {};
		snprintf(buf, MAX_NUMBER_SIZE, "%f", v);
		return Add(buf);
	}

	inline int32 String::FindFirstOf(const char& c) const
	{
		const int32 l = GetLength();
		for (int32 i = 0; i < l; i++)
		{
			if (data[i] == c)
				return i;
		}

		return -1;
	}

	inline int32 String::FindLastOf(const char& c) const
	{
		const int32 l = GetLength();
		for (int32 i = l; i >= 0; i--)
		{
			if (data[i] == c)
				return i;
		}

		return -1;
	}

	inline String String::SubStr(int32 fromIndex) const
	{
		int32 l = GetLength();
		Assert(fromIndex >= 0 && fromIndex < l, "SubStr range invalid");

		String result = "";
		for (int32 i = fromIndex; i < l; i++)
		{
			result.Add(data[i]);
		}

		return result;
	}

	inline String String::SubStr(int32 startIndex, int32 endIndex) const
	{
		int32 l = GetLength();
		Assert(startIndex >= 0 && startIndex < l, "SubStr range invalid");
		Assert(endIndex >= 0 && endIndex < l, "SubStr range invalid");
		Assert(startIndex < endIndex, "SubStr range invalid");

		String result = "";
		for (int32 i = startIndex; i < endIndex; i++)
		{
			result.Add(data[i]);
		}

		return result;
	}

	inline void String::Replace(const char& c, const char& replaceWith)
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

	inline void String::RemoveCharacter(const int32& removeIndex)
	{
		const int32 l = GetLength();
		Assert(removeIndex >= 0 && removeIndex < l, "String, invalid index");

		for (int32 i = removeIndex; i < l; i++)
		{
			data[i] = data[i + 1];
		}

		SetLength(l - 1);
	}

	inline void String::RemoveWhiteSpace()
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

	inline bool32 String::Contains(const String& str)
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

	inline bool32 String::StartsWith(const String& str) const
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

	inline void String::CopyFrom(const String& src, const int32& start, const int32& end)
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

	inline void String::ToUpperCase()
	{
		int32 l = GetLength();
		for (int32 i = 0; i < l; i++)
		{
			data[i] = (char)toupper(data[i]);
		}
	}

	inline int32 String::ToInt32() const
	{
		int32 result = atol(data);

		return result;
	}

	inline uint64 String::ToUint64() const
	{
		uint64 result = std::stoull(data);

		return result;
	}

	inline real32 String::ToReal32() const
	{
		real32 result = (real32)atof(data);

		return result;
	}

	inline ManagedArray<String> String::Split(const char& delim) const
	{
		ManagedArray<String> result = ManagedArray<String>(32, MemoryType::TRANSIENT);

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

	inline ManagedArray<String> String::Split(const int32& splitIndex) const
	{
		ManagedArray<String> result = ManagedArray<String>(32, MemoryType::TRANSIENT);

		const int32 len = GetLength();

		int32 start = 0;
		int32 end = 0;
		for (; end < len; end++)
		{
			if (end == splitIndex)
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

	inline String::String()
	{
		Clear();
		SetLength(0);
	}

	inline String::String(const char& c)
	{
		Clear();
		data[0] = c;
		SetLength(1);
	}

	inline String::String(const char* str)
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

	inline bool String::operator==(const char* other)
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

	inline bool String::operator==(const String& other) const
	{

		int32 l1 = GetLength();
		int32 l2 = other.GetLength();

		if (l1 != l2)
			return false;

		for (int32 i = 0; i < l1; i++)
		{
			if (data[i] != other.data[i])
				return false;
		}

		return true;
	}

}


