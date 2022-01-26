#pragma once
#include <SolarEngine.h>
#include "lightmapper/LightMapping.h"
#include "processors/AssetPacking.h"

namespace sol
{
	class EditorWindow
	{
	public:
		EditorWindow(const String& name, bool8 show) : name(name), show(show) {};
		virtual bool8 ShouldShow() { return show; };
		virtual String GetName() { return name; }
		virtual bool8 Show() = 0;

	protected:
		String name;
		bool8 show;
	};

	class EditorWindowList : public EditorWindow
	{
	public:
		EditorWindowList();
		virtual bool8 Show() override;
		bool8 Add(std::shared_ptr<EditorWindow> window);

	private:
		std::vector<std::shared_ptr<EditorWindow>> windows;
	};

	class TextureMetaFileWindow : public EditorWindow
	{
	public:
		TextureMetaFileWindow(const String& name);
		virtual bool8 Show() override;

	private:
		String path;
		TextureMetaFile file;
	};

	class EditorPerformanceWindow : public EditorWindow
	{
	public:
		EditorPerformanceWindow() : EditorWindow("Performance", true) {}
		virtual bool8 Show() override;
	private:
		real32 frameTimes[256] = {};
		real32 minTime = REAL_MAX;
		real32 maxTime = REAL_MIN;
	};

	class EditorRoomSettingsWindow : public EditorWindow
	{
	public:
		EditorRoomSettingsWindow(Room* room) : EditorWindow("Room", true), room(room) {}
		virtual bool8 Show() override;
	private:
		Room* room;
	};

	struct EditorState
	{
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