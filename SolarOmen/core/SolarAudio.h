#pragma once
#include "Defines.h"
#include "SolarString.h"
#include "SolarPlatform.h"

namespace cm
{
	namespace Audio
	{
		bool32 Initialize();
		void Shutdown();
	}

	void PlaySound(const CString& name, bool32 loop);
}

