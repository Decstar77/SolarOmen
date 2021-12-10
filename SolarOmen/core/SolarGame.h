#pragma once
#include "core/Defines.h"

namespace cm
{
	namespace Game
	{
		bool32 Initialize();
		void UpdateGame(real32 dt);
		void ConstructRenderGroup(EntityRenderGroup* renderGroup);
		void Shutdown();
	}
}