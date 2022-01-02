#pragma once
#include "Core.h"

namespace cm
{
	class Serializable;
	class BinaryFile
	{
	public:
		template<typename T>
		inline void Write(const std::vector<T>& v)
		{
			WritePrimitive((uint32)v.size());
			if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>)
			{
				for (const T& vv : v) { WritePrimitive(vv); }
			}
			else if constexpr (std::is_base_of_v<Serializable, T>)
			{
				for (const T& vv : v) { Write(reinterpret_cast<const Serializable*>(&vv)); }
			}
			else
			{
				for (const T& vv : v) { Write(vv); }
			}
		}

		void Write(const uint64& v);
		void Write(const uint32& v);
		void Write(const bool32& v);
		void Write(const uint8& v);
		void Write(const Vec2f& v);
		void Write(const Vec3f& v);
		void Write(const Vec4f& v);
		void Write(const CString& str);
		void Write(const Serializable* serializable);

		void SaveToDisk(const CString& path);

	private:
		std::vector<char> fileBytes;

		template<typename T>
		inline void WritePrimitive(T data)
		{
			static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Generic Write only supports primitive data types");

			char bytes[sizeof(T)] = {};
			std::copy(static_cast<const char*>(static_cast<const void*>(&data)),
				static_cast<const char*>(static_cast<const void*>(&data)) + sizeof(T), bytes);

			for (uint32 i = 0; i < (uint32)sizeof(T); i++)
				fileBytes.push_back(bytes[i]);
		}
	};

	class Serializable
	{
	public:
		virtual void SaveBinaryData(BinaryFile* file) const = 0;
	};

	class BinaryFileReader
	{
	public:
		BinaryFileReader();
		std::vector<char> Read(const CString& path);
		std::vector<char> fileBytes;
	};

}
