#pragma once

#include "Defines.h"
#include "SolarTypes.h"

namespace cm
{
	enum class MemberVariableType
	{
		INVALID = 0,
		STRING,
		ASSETID,
		UINT8,
		UINT16,
		UINT32,
		UINT64,
		INT8,
		INT16,
		INT32,
		INT64,
		BOOL32,
		REAL32,
		REAL64,
		VEC2F,
		VEC3F,
		VEC4F,
		MAT2F,
		MAT3F,
		MAT4F,
		STRUCT_WITH_TO_STRING,
	};

	class MemberVariable
	{
	public:
		CString name;
		MemberVariableType type;
		uint32 offset;
		MemberVariable(const CString& name, MemberVariableType type, uint32 offset) : name(name), type(type), offset(offset) {}
	};

#define INTROSPECTION_HEADER(clss) static MemberVariable INTROSPECTED_VARIABLES[]; inline static const char * CLASS_NAME = #clss;
#define INTROSPECT_MEMBER(clss, name) MemberVariable(#name, GetMemberVariableType(((clss*)0)->name), offsetof(clss, name)) 
#define INTROSPECT_VARIABLES(clss, ...) inline MemberVariable clss::INTROSPECTED_VARIABLES[] = {__VA_ARGS__}

	template<typename T>
	void PrintIntrospectedVairableNames()
	{
		for (int32 i = 0; i < ArrayCount(T::INTROSPECTED_VARIABLES); i++)
			LOG(T::INTROSPECTED_VARIABLES[i].name.GetCStr());
	}

	template<typename T>
	void PrintIntrospectedVairableData()
	{
		for (int32 i = 0; i < ArrayCount(T::INTROSPECTED_VARIABLES); i++)
		{
			LOG("Name: " << T::INTROSPECTED_VARIABLES[i].name.GetCStr() <<
				" T:" << (uint32)T::INTROSPECTED_VARIABLES[i].type <<
				" O:" << (uint32)T::INTROSPECTED_VARIABLES[i].offset);
		}
	}

	template<typename T>
	inline MemberVariableType GetMemberVariableType(const T& t)
	{
		CString test = t.ToString();
		return MemberVariableType::STRUCT_WITH_TO_STRING;
	}

	template<> inline MemberVariableType GetMemberVariableType<AssetId>(const AssetId& t) { return MemberVariableType::ASSETID; }
	template<> inline MemberVariableType GetMemberVariableType<CString>(const CString& t) { return MemberVariableType::STRING; }
	template<> inline MemberVariableType GetMemberVariableType<int32>(const int32& t) { return MemberVariableType::INT32; }
	template<> inline MemberVariableType GetMemberVariableType<uint32>(const uint32& t) { return MemberVariableType::UINT32; }
	template<> inline MemberVariableType GetMemberVariableType<uint64>(const uint64& t) { return MemberVariableType::UINT64; }
	template<> inline MemberVariableType GetMemberVariableType<real32>(const real32& t) { return MemberVariableType::REAL32; }
	template<> inline MemberVariableType GetMemberVariableType<Vec2f>(const Vec2f& t) { return MemberVariableType::VEC2F; }
	template<> inline MemberVariableType GetMemberVariableType<Vec3f>(const Vec3f& t) { return MemberVariableType::VEC3F; }
	template<> inline MemberVariableType GetMemberVariableType<Vec4f>(const Vec4f& t) { return MemberVariableType::VEC4F; }
}

