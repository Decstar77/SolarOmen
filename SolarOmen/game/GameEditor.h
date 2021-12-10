#pragma once
#include "core/SolarCore.h"

namespace cm
{
#define GetEditorState() EditorState *es = EditorState::Get()
	struct EditorState
	{
		bool showPerformanceWindow;
		bool showConsoleWindow;
		bool showRoomWindow;
		bool showInspectorWindow;
		bool showAssetWindow;
		bool showRenderSettingsWindow;

		real32 frameTimes[256] = {};
		real32 minTime = REAL_MAX;
		real32 maxTime = REAL_MIN;

		inline static EditorState* Get() { return editorState; }
		inline static void Initialize(EditorState* es) { editorState = es; }
	private:
		inline static EditorState* editorState = nullptr;
	};
}