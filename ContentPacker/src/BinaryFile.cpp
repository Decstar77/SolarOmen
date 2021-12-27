#include "BinaryFile.h"
#include<fstream>

namespace cm
{
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

	void BinaryFile::SaveToDisk(const CString& path)
	{
		std::ofstream file(path.GetCStr(), std::ios::out | std::ios::binary);
		file.write((const char*)fileBytes.data(), fileBytes.size());
		file.close();
	}
}