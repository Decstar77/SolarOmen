#pragma once
#include <SolarEngine.h>

namespace sol
{
	struct EditorState
	{
		bool8 showPerformanceWindow;
		bool8 showConsoleWindow;
		bool8 showRoomWindow;
		bool8 showInspectorWindow;
		bool8 showAssetWindow;
		bool8 showRenderSettingsWindow;
		bool8 showBuildWindow;

		bool8 isLightMapping;

		uint32 raytraceUpdateCounter;
		std::vector<Vec4f> raytracePixels;
		TextureHandle raytracedTexture;

		real32 frameTimes[256] = {};
		real32 minTime = REAL_MAX;
		real32 maxTime = REAL_MIN;

		Camera camera;

		Room room;
		Entity selectedEntity;
	};
}