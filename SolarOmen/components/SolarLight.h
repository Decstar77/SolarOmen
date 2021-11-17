#pragma once

#include "core/SolarCore.h"

namespace cm
{
	class LightType
	{
	public:
		enum class Value
		{
			POINT = 0,
			DIRECTIONAL,
			SPOT,
			AREA,
			COUNT,
		};

		LightType();
		LightType(Value v);
		CString ToString() const;
		static LightType ValueOf(const uint32& v);
		static LightType ValueOf(const CString& str);
		inline bool operator==(const LightType& rhs) const { return this->value == rhs.value; }
		inline bool operator!=(const LightType& rhs) const { return this->value != rhs.value; }
		inline operator uint32() const { return (uint32)value; }

	private:
		Value value;

		inline static CString __STRINGS__[] = {

			"POINT",
			"DIRECTIONAL",
			"SPOT",
			"AREA"
		};
	};

	struct LightComponent
	{
		bool32 active;
		LightType type;
		Vec3f colour;
		real32 intensity;
	};
}
