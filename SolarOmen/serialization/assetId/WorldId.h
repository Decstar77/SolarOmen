#pragma once
#include "core/SolarCore.h"
namespace cm
{
	class WorldId
	{
	public:
		enum class Value
		{
			INVALID = 0,
			// @NOTE: External
			DEMO,
			COUNT,
		};

		WorldId()
		{
			value = Value::INVALID;
		}

		WorldId(Value v)
		{
			this->value = v;
		}

		inline bool32 IsValid() const
		{
			return this->value != Value::INVALID && this->value != Value::COUNT;
		}

		inline CString ToString() const
		{
			CString copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline static WorldId ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid world id");
			return (WorldId::Value)v;
		}

		inline static WorldId ValueOf(const CString& str)
		{
			uint32 count = (uint32)Value::COUNT;
			for (uint32 i = 0; i < count; i++)
			{
				if (str == __STRINGS__[i])
				{
					return ValueOf(i);
				}
			}

			return Value::INVALID;
		}

		inline bool operator==(const WorldId& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const WorldId& rhs) const
		{
			return this->value != rhs.value;
		}

		inline operator uint32() const
		{
			return (uint32)value;
		}

	private:
		Value value;

		inline static const CString __STRINGS__[] = {
			"INVALID",
			"DEMO",
			"COUNT"
		};
	};

}