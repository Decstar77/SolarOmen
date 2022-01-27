#pragma once
#include <SolarEngine.h>

#include "EditorWindows.h"

namespace sol
{
	class EditorState
	{
	public:
		bool8 showPerformanceWindow;
		bool8 showConsoleWindow;
		bool8 showInspectorWindow;
		bool8 showAssetWindow;
		bool8 showRenderSettingsWindow;
		bool8 showBuildWindow;

		bool8 isLightMapping;

		EditorWindowList windows;
		ReferenceRayTracer referenceRayTracer;

		real32 frameTimes[256] = {};
		real32 minTime = REAL_MAX;
		real32 maxTime = REAL_MIN;

		Camera camera;

		Room room;
		Entity selectedEntity;
	};
}