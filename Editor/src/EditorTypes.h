#pragma once
#include <SolarEngine.h>

#include "EditorWindows.h"
#include <stack>
#include <functional>

namespace sol
{
	class EditorAction
	{
	public:
		virtual bool8 Redo(EditorState* es) = 0;
		virtual bool8 Undo(EditorState* es) = 0;
	};

	class EditorSelectionAction : public EditorAction
	{
	public:
		EditorSelectionAction(const std::vector<Entity>& old, const std::vector<Entity>& cur) : old(old), cur(cur) {};

		virtual bool8 Redo(EditorState* es) override;
		virtual bool8 Undo(EditorState* es) override;

	private:
		std::vector<Entity> old;
		std::vector<Entity> cur;
	};

	class EditorSetParentAction : public EditorAction
	{
	public:
		EditorSetParentAction(Entity entity, Entity newParent, Entity oldParent)
			: entity(entity), newParent(newParent), oldParent(oldParent) {}

		virtual bool8 Redo(EditorState* es) override;
		virtual bool8 Undo(EditorState* es) override;

	private:
		Entity entity;
		Entity newParent;
		Entity oldParent;
	};

	class EditorSetNameAction : public EditorAction
	{
	public:
		EditorSetNameAction(Entity entity, String oldName, String curName)
			: entity(entity), oldName(oldName), curName(curName) {}

		virtual bool8 Redo(EditorState* es) override;
		virtual bool8 Undo(EditorState* es) override;

	private:
		Entity entity;
		String oldName;
		String curName;
	};

	template<typename T>
	class EditorStateChangeAction : public EditorAction
	{
	public:
		using SetFunc = std::function<bool8(EditorState*, T*)>;

		EditorStateChangeAction(const T& oldState, const T& newState, const SetFunc& setter)
			: oldState(std::make_shared<T>(oldState)), newState(std::make_shared<T>(newState)) {};

		EditorStateChangeAction(const std::shared_ptr<T>& oldState, const std::shared_ptr<T>& newState, const SetFunc& setter)
			: oldState(oldState), newState(newState), setter(setter) {};

		virtual bool8 Redo(EditorState* es) override {
			return setter(es, newState->get());
		}

		virtual bool8 Undo(EditorState* es) override {
			return setter(es, oldState->get());
		};

	private:
		SetFunc setter;
		std::shared_ptr<T> oldState;
		std::shared_ptr<T> newState;
	};

	class EditorUndoSystem
	{
	public:
		inline void AddAction(const std::shared_ptr<EditorAction>& action) {
			undoActions.push(action);
			if (!redoActions.empty()) {
				redoActions = std::stack<std::shared_ptr<EditorAction>>();
			}
		}

		inline bool8 Redo(EditorState* es) {
			if (!redoActions.empty()) {
				auto action = redoActions.top();
				redoActions.pop();

				undoActions.push(action);

				return action->Redo(es);
			}

			return true;
		}

		inline bool8 Undo(EditorState* es) {
			if (!undoActions.empty()) {
				auto action = undoActions.top();
				undoActions.pop();

				redoActions.push(action);

				return action->Undo(es);
			}

			return true;
		}

	private:
		std::stack<std::shared_ptr<EditorAction>> redoActions;
		std::stack<std::shared_ptr<EditorAction>> undoActions;
	};

	class EditorSelection
	{
	public:
		EditorSelection(EditorUndoSystem* undoSystem) : undoSystem(undoSystem), selectedEntities(0) {}

		inline void Set(Entity entity) {
			auto old = selectedEntities;
			selectedEntities.clear();
			selectedEntities.push_back(entity);
			undoSystem->AddAction(std::make_shared<EditorSelectionAction>(old, selectedEntities));
		}

		inline void Set(const std::vector<Entity>& selection) {
			selectedEntities = selection;
		}

		inline std::vector<Entity> GetSelectedEntities() {
			return selectedEntities;
		}
		//inline void Add(Entity entity) {
		//	selectedEntities.push_back(entity);
		//}

	private:
		std::vector<Entity> selectedEntities;
		EditorUndoSystem* undoSystem;
	};

	class EditorState
	{
	public:
		bool8 showPerformanceWindow;
		bool8 showConsoleWindow;
		bool8 showAssetWindow;
		bool8 showRenderSettingsWindow;
		bool8 showBuildWindow;

		bool8 isLightMapping;

		EditorWindowList windows;
		ReferenceRayTracer referenceRayTracer;
		EditorUndoSystem undoSystem;
		EditorSelection selection;

		real32 frameTimes[256] = {};
		real32 minTime = REAL_MAX;
		real32 maxTime = REAL_MIN;

		Camera camera;

		Room room;
	public:
		EditorState() : selection(&undoSystem) {}
	};
}