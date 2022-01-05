#pragma once
#include "SolarDefines.h"
#include "SolarResourceTypes.h"

namespace sol
{
	class ResourceSystem
	{
	public:
		static bool8 Initialize();
		static void Shutdown();
		static void LoadAllShaderPrograms();
		static void LoadAllModels();
	};
}