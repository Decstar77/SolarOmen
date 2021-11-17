#include "SolarLight.h"

namespace cm
{
	LightType::LightType()
	{
		value = Value::POINT;
	}

	LightType::LightType(Value v)
	{
		this->value = v;
	}

	CString LightType::ToString() const
	{
		CString copy = __STRINGS__[(uint32)value];

		return copy;
	}

	LightType LightType::ValueOf(const uint32& v)
	{
		Assert(v < (uint32)Value::COUNT, "Invalid model id");
		return (LightType::Value)v;
	}

	LightType LightType::ValueOf(const CString& str)
	{
		uint32 count = (uint32)Value::COUNT;
		for (uint32 i = 0; i < count; i++)
		{
			if (str == __STRINGS__[i])
			{
				return ValueOf(i);
			}
		}

		return Value::POINT;
	}

}