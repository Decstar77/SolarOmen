#pragma once
#include "core/SolarCore.h"

namespace cm
{
	class ShaderId
	{
	public:
		enum class Value
		{
			INVALID,
			BASIC_PBR,
			BLOOMFILTER,
			DEBUG_LINE,
			DEFFERD_PHONG,
			DEPTH_POINT_LIGHTS,
			EQUIRECTANGULAR_TO_CUBEMAP,
			IRRADIANCE_CONVOLUTION,
			DEPTH_ONLY,
			GAUSSIAN_BLUR_HORIZONTAL,
			GAUSSIAN_BLUR_VERTICAL,
			GBUFFER,
			PARTICLE,
			PHONG,
			POST_PROCESSING,
			DEPTH_REDUCTION,
			DEPTH_REDUCTION_DOWN,
			SHADOW_RAY,
			SKYBOX,
			TEST_SHPERE_GEN,
			UNLIT,
			COUNT,
		};

		ShaderId()
		{
			value = Value::INVALID;
		}

		ShaderId(Value v)
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

		inline static ShaderId ValueOf(const uint32& v)
		{
			Assert(v < (uint32)Value::COUNT, "Invalid shader id");
			return (ShaderId::Value)v;
		}

		inline static ShaderId ValueOf(const CString& str)
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

		inline bool operator==(const ShaderId& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const ShaderId& rhs) const
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
			"BASIC_PBR",
			"BLOOMFILTER",
			"DEBUG_LINE",
			"DEFFERD_PHONG",
			"DEPTH_POINT_LIGHTS",
			"EQUIRECTANGULAR_TO_CUBEMAP",
			"IRRADIANCE_CONVOLUTION",
			"DEPTH_ONLY",
			"GAUSSIAN_BLUR_HORIZONTAL",
			"GAUSSIAN_BLUR_VERTICAL",
			"GBUFFER",
			"PARTICLE",
			"PHONG",
			"POST_PROCESSING",
			"DEPTH_REDUCTION",
			"DEPTH_REDUCTION_DOWN",
			"SHADOW_RAY",
			"SKYBOX",
			"TEST_SPHERE_GEN",
			"UNLIT",
		};
	};
}

