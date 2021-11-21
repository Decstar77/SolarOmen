#pragma once
#include "core/SolarCore.h"
namespace cm
{
	class ModelId
	{
	public:
		enum class Value : uint32
		{
			INVALID = 0,
			// @NOTE: Internal
			SCREEN_SPACE_QUAD,
			// @NOTE: External
			CUBE,
			PLANE,
			SM_WEP_RIFLE_BASE_01,
			SM_BLD_SECTION_DOOR_02,
			SM_VEH_CLASSIC_01,
			CM_TRACK_OVAL,
			CORRIDOR_01,
			BOOMBOX,
			FLOORTILE_EMPTY,
			TEST_1,
			COUNT,
		};

		ModelId()
		{
			value = Value::INVALID;
		}

		ModelId(Value v)
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

		inline static ModelId ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid model id");
			return (ModelId::Value)v;
		}

		inline static ModelId ValueOf(const CString& str)
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

		inline bool operator==(const ModelId& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const ModelId& rhs) const
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
			"SCREEN_SPACE_QUAD",
			"CUBE",
			"PLANE",
			"SM_WEP_RIFLE_BASE_01",
			"SM_BLD_SECTION_DOOR_02",
			"SM_VEH_CLASSIC_01",
			"CM_TRACK_OVAL",
			"CORRIDOR_01",
			"BOOMBOX",
			"FLOORTILE_EMPTY",
			"TEST_1",
			"COUNT"
		};
	};
}
