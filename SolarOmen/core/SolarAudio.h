#pragma once
#include "Defines.h"
#include "SolarString.h"

namespace cm
{
	namespace Audio
	{
		bool32 Initialize();
		void Shutdown();
	}

	void PlaySound(const CString& path, bool32 loop);
}

