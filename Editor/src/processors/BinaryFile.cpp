#include "BinaryFile.h"
#include<fstream>

namespace sol
{
	void BinaryFile::Write(const ResourceId& v)
	{
		WritePrimitive(v.number);
	}

	void BinaryFile::Write(const uint64& v)
	{
		WritePrimitive(v);
	}

	void BinaryFile::Write(const uint32& v)
	{
		WritePrimitive(v);
	}

	void BinaryFile::Write(const bool32& v)
	{
		WritePrimitive(v);
	}

	void BinaryFile::Write(const uint8& v)
	{
		WritePrimitive(v);
	}

	void BinaryFile::Write(const Vec2f& v)
	{
		WritePrimitive(v.x);
		WritePrimitive(v.y);
	}

	void BinaryFile::Write(const Vec3f& v)
	{
		WritePrimitive(v.x);
		WritePrimitive(v.y);
		WritePrimitive(v.z);
	}

	void BinaryFile::Write(const Vec4f& v)
	{
		WritePrimitive(v.x);
		WritePrimitive(v.y);
		WritePrimitive(v.z);
		WritePrimitive(v.w);
	}

	void BinaryFile::Write(const String& str)
	{
		WritePrimitive(str.GetLength());
		for (int32 i = 0; i < str.GetLength(); i++) { WritePrimitive(str[i]); }
	}

	void BinaryFile::Write(const Serializable* serializable)
	{
		serializable->SaveBinaryData(this);
	}

	void BinaryFile::SaveToDisk(const String& path)
	{
		std::ofstream file(path.GetCStr(), std::ios::out | std::ios::binary);
		file.write((const char*)fileBytes.data(), fileBytes.size());
		file.close();
	}

	BinaryFileReader::BinaryFileReader()
	{

	}

	std::vector<char> BinaryFileReader::Read(const String& path)
	{
		fileBytes.clear();

		std::ifstream file(path.GetCStr(), std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			std::streamsize size = file.tellg();
			fileBytes.resize(size);
			file.seekg(0, std::ios::beg);
			if (!file.read(fileBytes.data(), size))
			{
				Assert(0, "INVALID BINARY FILE READ");
			}
		}

		file.close();

		return fileBytes;
	}
}