#pragma once

#include <SolarEngine.h>
#include "EditorTypes.h"

namespace sol
{
	bool8 InitialzieImGui();
	void UpdateImGui(EditorState* es, real32 dt);
}