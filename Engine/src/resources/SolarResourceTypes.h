#pragma once
#include "../SolarDefines.h"
#include "../core/SolarString.h"
#include "../core/SolarMath.h"

namespace sol
{
	struct SOL_API ResourceId
	{
		union
		{
			uint64 number;
			char chars[8];
			static_assert(sizeof(uint64) == sizeof(char[8]));
		};

		inline String ToString() const { return String(chars); }
		inline bool operator==(const ResourceId& rhs) const { return this->number == rhs.number; }
		inline bool operator!=(const ResourceId& rhs) const { return this->number != rhs.number; }
		inline bool IsValid() const { return number != 0; }
		inline operator uint64() const { return number; }
	};

	struct SOL_API ModelResource
	{
		String name;
	};

	class SOL_API Resources
	{
	public:
		static ModelResource* GetModelResource(const ResourceId& name);
		static ModelResource* GetModelResource(const String& name);
	};
}