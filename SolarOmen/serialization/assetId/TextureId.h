#pragma once
#include "core/SolarCore.h"
namespace cm
{
	class TextureId
	{
	public:
		enum class Value
		{
			INVALID = 0,
			// @NOTE: External
			POLYGONSCIFI_01_C,
			BOOMBOX_BASECOLOR,
			BOOMBOX_OCCLUSIONROUGHNESSMETALLIC,
			BOOMBOX_NORMAL,
			BOOMBOX_EMISSIVE,
			COLOUR_PALETTE_SET_0,

			NEWPORT_LOFT,
			STUDIO_SMALL_08_4K,
			FS002_DAY_SUNLESS,

			ARIAL,

			COUNT,

			//@NOTE: Internal, shoud we be more descriptive ? Eg back buffer, etc ?
			INTERNAL,
		};

		TextureId()
		{
			value = Value::INVALID;
		}

		TextureId(Value v)
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

		inline static TextureId ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid texture id");
			return (TextureId::Value)v;
		}

		inline static TextureId ValueOf(const CString& str)
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

		inline bool operator==(const TextureId& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const TextureId& rhs) const
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
			"POLYGONSCIFI_01_C",
			"BOOMBOX_BASECOLOR",
			"BOOMBOX_OCCLUSIONROUGHNESSMETALLIC",
			"BOOMBOX_NORMAL",
			"BOOMBOX_EMISSIVE",
			"COLOUR_PALETTE_SET_0",

			"NEWPORT_LOFT",
			"STUDIO_SMALL_08_4K",
			"FS002_DAY_SUNLESS",

			"ARIAL",

			"COUNT"
			"INTERNAL",
		};
	};
}
