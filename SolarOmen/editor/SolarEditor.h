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



	class UndoAction
	{
	public:
		enum class Value
		{
			INVALID = 0,
			SELECTION_CHANGE,
			ENTITY_CHANGE,
			COUNT
		};

		UndoAction()
		{
			value = Value::INVALID;
		}

		UndoAction(Value v)
		{
			this->value = v;
		}

		inline CString ToString() const
		{
			CString copy = __STRINGS__[(uint32)value];

			return copy;
		}

		inline bool operator==(const UndoAction& rhs) const
		{
			return this->value == rhs.value;
		}

		inline bool operator!=(const UndoAction& rhs) const
		{
			return this->value != rhs.value;
		}

		inline operator uint32() const
		{
			return (uint32)value;
		}

		inline Value GetRawValue()
		{
			return value;
		}

	private:
		Value value;
		inline static CString __STRINGS__[] = {
			"UNKNOWN",
			"SELECTION_CHANGE",
			"ENTITY_CHANGE"
		};
	};

	class UndoSelectionChange
	{
	public:
		EntityId current;
		EntityId previous;
	};

	class UndoEntityChange
	{
	public:
		Entity current;
		Entity previous;
	};

	class UndoEntry
	{
	public:
		UndoAction action;

		union
		{
			UndoSelectionChange selectionChange;
			UndoEntityChange entityChange;
		};

		UndoEntry()
		{
			ZeroStruct(this);
		};
	};

	struct UndoSystem
	{
	private:
		inline static const int32 UNDO_AMOUNT = 10;
		UndoEntry undoStorage[UNDO_AMOUNT];
		UndoEntry redoStorage[UNDO_AMOUNT];

	public:
		CircularQueue<UndoEntry> undoStates;
		CircularQueue<UndoEntry> redoStates;

		inline void Initialize()
		{
			undoStates = CircularQueue<UndoEntry>(undoStorage, UNDO_AMOUNT);
			redoStates = CircularQueue<UndoEntry>(redoStorage, UNDO_AMOUNT);
		}

		inline void Do(const UndoEntry& t)
		{
			undoStates.Push(t);
		}

		inline UndoEntry Undo()
		{
			if (undoStates.count > 0)
			{
				UndoEntry entry = undoStates.Pop();
				redoStates.Push(entry);
				return entry;
			}
			return {};
		}

		inline UndoEntry Redo()
		{
			if (redoStates.count > 0)
			{
				UndoEntry entry = redoStates.Pop();
				undoStates.Push(entry);
				return entry;
			}
			return {};
		}
	};

	struct EditorState
	{
		bool32 inGame;
		Camera camera;

		bool32 vsync;
		real32 frameTimes[256];

		bool32 showWorldWindow;
		bool32 showPerformanceWindow;
		bool32 showRenderSettingsWindow;
		bool32 showDebugWindow;

		WorldId currentWorld;
		Entity tempEntity;
		Entity* selectedEntity;

		UndoSystem undoSystem;

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
