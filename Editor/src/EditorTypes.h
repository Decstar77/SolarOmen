#pragma once
#include <SolarEngine.h>

namespace sol
{
	struct EditorState
	{
		bool showPerformanceWindow;
		bool showConsoleWindow;
		bool showRoomWindow;
		bool showInspectorWindow;
		bool showAssetWindow;
		bool showRenderSettingsWindow;
		bool showBuildWindow;

		real32 frameTimes[256] = {};
		real32 minTime = REAL_MAX;
		real32 maxTime = REAL_MIN;

		Camera camera;
	};
}