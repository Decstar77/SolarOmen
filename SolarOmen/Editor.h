#pragma once
#if 0
#include "platform/SolarPlatform.h"
#include "renderer/SolarRenderer.h"
#include "SolarInput.h"
#include "SolarOmen.h"
#include "SolarInput.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_win32.h"
#include "vendor/imgui/imgui_impl_dx11.h"
#include "vendor/imguizmo/ImGuizmo.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace cm
{
	enum class EditorMode
	{
		GAME = 0,
		FREE = 1,
	};

	enum class EditorAction
	{
		UNKNOWN,
		SELECTION_CHANGE,
		GAMESTATE_CHANGE,
	};

	template<typename T>
	struct UndoSystem
	{
	private:
		inline static const int32 UNDO_AMOUNT = 10;
	public:

		int32 undoEnd;
		int32 undoStart;
		int32 undoCurrent;
		T undoStates[UNDO_AMOUNT + 2];

		inline void Init(T* initalState)
		{
			undoEnd = 1;
			undoStart = 0;
			undoCurrent = 0;

			Do(initalState);
		}

		inline T* GetCurrent()
		{
			return &undoStates[undoCurrent];
		}

		inline void Redo(T* gs)
		{
			undoCurrent = IncAndWrapValue(undoCurrent);
			if (undoCurrent == undoEnd)
			{
				undoCurrent--;
			}

			*gs = undoStates[undoCurrent];
		}

		inline void Do(T* gs)
		{
			undoCurrent = IncAndWrapValue(undoCurrent);

			if (undoCurrent == undoEnd)
			{
				undoEnd = IncAndWrapValue(undoEnd);
				if (undoEnd == undoStart)
				{
					undoStart = IncAndWrapValue(undoStart);
				}
			}

			undoStates[undoCurrent] = *gs;
		}

		inline T* Undo()
		{
			undoCurrent = DecAndWrapValue(undoCurrent);
			if (undoCurrent == undoStart)
			{
				undoCurrent++;
			}

			return &undoStates[undoCurrent];
		}

	private:
		inline int32 DecAndWrapValue(int32 v) {
			v--;
			if (v < 0)
			{
				v = ArrayCount(undoStates) - 1;
			}

			return v;
		}

		inline int32 IncAndWrapValue(int32 v) {
			v++;
			if (v > ArrayCount(undoStates) - 1)
			{
				v = 0;
			}

			return v;
		}
	};

	struct EditorState
	{
		static_assert(sizeof(UndoSystem<GameState>) < Megabytes(256), "undo system getting big");

		Camera camera;
		bool vsync;
		bool show_bounding_boxes;
		bool show_colliders;
		bool windowOpen;
		bool physicsWindowOpen;

		EditorMode mode;

		CString currentGameFile;

		real32 frameTimes[128];
		real32 startUpTime;

		int32 temporayItemsCount;
		char* temporayItemsList[512];

		Entity clipBoardEntity;

		UndoSystem<EditorAction> actions;
		UndoSystem<GameState> gameStateUndo;
		UndoSystem<Entity*> selectionUndo;

		// @TODO: More than one
		OBB* selectedCollider;
		Entity* selected_entity;
		EntityPrefab* selectedPrefab;

		ImGuizmo::OPERATION op;
		ImGuizmo::MODE md;
	};

	void BeginEditorNewFrame(EditorState* es);

	void DrawEditorFrame(EditorState* es);

	void OperateCamera(EditorState* es, Input* input);

	void OperateGizmo(EditorState* es, GameState* gs, PlatformState* ws, Input* input);

	Entity* SelectEntity(EditorState* es, GameState* gs, RenderState* rs, AssetState* as, PlatformState* ws, Input* input);

	void InitializeEditorState(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, PlatformState* ws);

	void RenderEditor(EditorState* es);

	void UpdateEditor(GameState* gs, RenderState* rs, AssetState* as, EditorState* es, Input* input, PlatformState* ws, TransientState* ts);
}
#endif