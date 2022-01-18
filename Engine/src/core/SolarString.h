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
#include "../SolarDefines.h"
#include "SolarContainers.h"

namespace sol
{
	class SOL_API String
	{
	public:
		// @TODO @BUG @CHECK: If the buffer is full there will be no terminating character at the end of the string!!
		// @NOTE: The length of the string does not include the null char
		// @NOTE: Last char is treated as an unsigned int to store the length
		inline static const int32 MAX_NUMBER_SIZE = 22;
		inline static const int32 TOTAL_SIZE = 256;
		inline static const int32 CAPCITY = TOTAL_SIZE - 2;

	public:

		int32 GetLength() const;
		const char* GetCStr() const;
		char* GetCStr();
		void Clear();
		String& Add(const char& c);
		String& Add(const char* c);
		String& Add(const String& c);
		String& Add(const int32& v);
		String& Add(const uint32& v);
		String& Add(const int64& v);
		String& Add(const uint64& v);
		String& Add(const real32& v);
		int32 FindFirstOf(const char& c) const;
		int32 FindLastOf(const char& c) const;
		String SubStr(int32 fromIndex) const;
		String SubStr(int32 startIndex, int32 endIndex) const;
		void Replace(const char& c, const char& replaceWith);
		void RemoveCharacter(const int32& removeIndex);
		void RemoveWhiteSpace();
		bool32 Contains(const String& str);
		bool32 StartsWith(const String& str) const;
		void CopyFrom(const String& src, const int32& start, const int32& end);
		void ToUpperCase();
		int32 ToInt32() const;
		uint64 ToUint64() const;
		real32 ToReal32() const;

		ManagedArray<String> Split(const char& delim) const;
		ManagedArray<String> Split(const int32& splitIndex) const;

		String();
		explicit String(const char& c);
		String(const char* str);

		inline char& operator[](const int32& index) { Assert(index >= 0 && index < GetLength(), "String, invalid index"); return data[index]; }
		inline char operator[](const int32& index) const { Assert(index >= 0 && index < GetLength(), "String, invalid index"); return data[index]; }
		inline bool operator==(const char* other);
		inline bool operator==(const String& other) const;
		inline bool operator!=(const String& other) const { return !(*this == other); }

	private:
		char data[TOTAL_SIZE];
		inline void SetLength(const int32 l) { data[CAPCITY + 1] = (uint8)l; }
	};

	namespace Util
	{
		inline String StripFilePath(const String& str)
		{
			int32 f = str.FindLastOf('/');
			int32 b = str.FindLastOf('\\');
			int32 index = f > b ? f : b;
			String result = str.SubStr(index + 1);
			return result;
		}

		inline String StripFileExtension(const String& str)
		{
			int32 l = str.GetLength();
			int32 index = -1;

			for (int32 i = l - 1; i >= 0; i--)
			{
				if (str[i] == '\\' || str[i] == '/')
					break;
				if (str[i] == '.')
					index = i;
			}

			String result = {};
			if (index >= 0)
			{
				result = str.SubStr(0, index);
			}

			return result;
		}

		inline String StripFilePathAndExtentions(const String& str)
		{
			String result = StripFilePath(str);
			result = StripFileExtension(result);

			return result;
		}

		inline String GetFileExtension(const String& str)
		{
			String file = str.SubStr(str.FindLastOf('/') + 1);
			String result = file.SubStr(file.FindFirstOf('.') + 1);

			return result;
		}
	};
}



