#pragma once
#include "core/SolarCore.h"

namespace cm
{
	class GameplaySettings
	{
	};

	class RenderingSettings
	{
		class ShadowQuality
		{
		public:
			enum class Value : uint8
			{
				LOW = 0,
				MEDIUM,
				HIGH,
				ULTRA,
			};

			ShadowQuality()
			{
				value = Value::HIGH;
			}

			ShadowQuality(Value v)
			{
				this->value = v;
			}

			inline int32 GetResolution()
			{
				switch (value)
				{
				case Value::LOW: return 512;
				case Value::MEDIUM: return 1024;
				case Value::HIGH: return 2048;
				case Value::ULTRA: return 4096;
				}
				return 0;
			}

			inline CString ToString()
			{
				return __STRINGS__[(uint32)value];
			}

			inline bool operator==(const ShadowQuality& rhs) const
			{
				return this->value == rhs.value;
			}

			inline bool operator!=(const ShadowQuality& rhs) const
			{
				return this->value != rhs.value;
			}

			inline operator uint32() const
			{
				return (uint32)value;
			}

		private:
			Value value;

			inline static const CString __STRINGS__[] =
			{
				"LOW",
				"MEDIUM",
				"HIGH",
				"ULTRA"
			};
		};

	public:
		inline static bool32 vsync = true;
		inline static ShadowQuality shadowQuality = ShadowQuality::Value::ULTRA;
	};
}

