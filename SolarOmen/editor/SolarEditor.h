#pragma once

#include "../SolarOmen.h"
#include "../renderer/SolarRenderer.h"



#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_win32.h"
#include "../vendor/imgui/imgui_impl_dx11.h"
#include "../vendor/imguizmo/ImGuizmo.h"
#include "../vendor/imgui_node/imnodes.h"

#include "NodeWindow.h"
#include "Gizmos.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace cm
{
	//class WorldWindow
	//{
	//public:
	//	void Initialize();
	//	void Show(Input* input, );
	//};

	struct EditorState
	{
		Camera camera;

		bool32 vsync;
		bool32 inGame;

		bool32 showPerformanceWindow;
		bool32 showRenderSettingsWindow;

		WorldId currentWorld;

		real32 frameTimes[256];

		Entity* selectedEntity;

		Gizmo gizmo;

		NodeWindow nodeWindow;
	};

	////////////////////////////////////////////////////
	// @NOTE: Platform layer call this
	////////////////////////////////////////////////////

	void InitializeEditorState(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, PlatformState* ws);

	void RenderEditor(EditorState* es);

	void UpdateEditor(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, Input* input,
		PlatformState* ws, EntityRenderGroup* renderGroup);
}
