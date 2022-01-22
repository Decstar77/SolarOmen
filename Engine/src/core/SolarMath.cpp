#include "SolarMath.h"
#include <math.h>
#include <random>

namespace sol
{
	real64 RandomReal64()
	{
		static std::uniform_real_distribution<real64> distribution(0.0, 1.0);
		static std::mt19937 generator;
		return distribution(generator);
	}

	real64 RandomReal64(real64 min, real64 max)
	{
		return min + (max - min) * RandomReal64();
	}

	real32 RandomReal32()
	{
		return (real32)RandomReal64();
	}

	real32 sol::RandomReal32(real32 min, real32 max)
	{
		return min + (max - min) * RandomReal32();
	}

	int64 RandomInt64(int64 min, int64 max)
	{
		return (int64)(RandomReal64((real64)min, (real64)(max + 1)));
	}

	int32 RandomInt32(int32 min, int32 max)
	{
		return (int32)RandomInt64((int64)min, (int64)max);
	}

	real32 Sin(const real32& rad)
	{
		return sinf(rad);
	}

	real64 Sin(const real64& rad)
	{
		return sin(rad);
	}

	real32 ArcSin(const real32& rad)
	{
		return asinf(rad);
	}

	real64 ArcSin(const real64& rad)
	{
		return asin(rad);
	}

	real32 Cos(const real32& rad)
	{
		return cosf(rad);
	}

	real64 Cos(const real64& rad)
	{
		return cos(rad);
	}

	real32 ArcCos(const real32& rad)
	{
		return acosf(rad);
	}

	real64 ArcCos(const real64& rad)
	{
		return acos(rad);
	}

	real32 Tan(const real32& rad)
	{
		return tanf(rad);
	}

	real64 Tan(const real64& rad)
	{
		return tan(rad);
	}

	real32 ATan2(const real32& y, const real32& x)
	{
		return atan2f(y, x);
	}

	real64 ATan2(const real64& y, const real64& x)
	{
		return atan2(y, x);
	}

	real32 Round(const real32& x)
	{
		return roundf(x);
	};

	real64 Round(const real64& x)
	{
		return round(x);
	};

	real32 Floor(const real32& x)
	{
		return floorf(x);
	}

	real64 Floor(const real64& x)
	{
		return floor(x);
	}

	real32 Ceil(const real32& x)
	{
		return ceilf(x);
	}

	real64 Ceil(const real64& x)
	{
		return ceil(x);
	}

	real32 Pow(const real32& base, const real32& exp)
	{
		return powf(base, exp);
	}

	real64 Pow(const real64& base, const real64& exp)
	{
		return pow(base, exp);
	}

	real32 Sqrt(const real32& x)
	{
		return sqrtf(x);
	}

	real64 Sqrt(const real64& x)
	{
		return sqrt(x);
	}
}