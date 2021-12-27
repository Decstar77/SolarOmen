#pragma once
#include "Core.h"

namespace cm
{
	class BinaryFile
	{
	public:
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

		void Write(const Vec2f& v);
		void Write(const Vec3f& v);

		void SaveToDisk(const CString& path);

	private:
		std::vector<char> fileBytes;
	};
}
