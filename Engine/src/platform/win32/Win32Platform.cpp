#include "platform/SolarPlatform.h"

#if SOLAR_PLATFORM_WINDOWS
namespace sol
{
	bool8 Platform::Intialize(int32 x, int32 y, int32 width, int32 height)
	{
		return bool8();
	}

	void Platform::Shutdown()
	{

	}
}
#endif