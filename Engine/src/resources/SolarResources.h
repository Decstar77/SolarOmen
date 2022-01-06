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
		static void LoadAllModels();
		static void LoadAllTextures();
		static void LoadAllShaderPrograms();
	};
}