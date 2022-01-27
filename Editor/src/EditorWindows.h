#pragma once
#include "Core.h"
#include "lightmapper/LightMapping.h"
#include "processors/AssetPacking.h"
#include "../vendor/imgui/imgui.h"
namespace sol
{
	class EditorState;

	class EditorWindow
	{
	public:
		EditorWindow(const String& name, bool8 show) : name(name), show(show) {};
		virtual bool8 ShouldShow() { return show; };
		virtual String GetName() { return name; }
		virtual bool8 Show(EditorState* es) = 0;

	protected:
		String name;
		bool8 show;
	};

	class EditorWindowList : public EditorWindow
	{
	public:
		EditorWindowList();
		virtual bool8 Show(EditorState* es) override;
		bool8 Add(std::shared_ptr<EditorWindow> window);

	private:
		std::vector<std::shared_ptr<EditorWindow>> windows;
	};

	class TextureMetaFileWindow : public EditorWindow
	{
	public:
		TextureMetaFileWindow(const String& name);
		virtual bool8 Show(EditorState* es) override;

	private:
		String path;
		TextureMetaFile file;
	};

	class EditorPerformanceWindow : public EditorWindow
	{
	public:
		EditorPerformanceWindow() : EditorWindow("Performance", true) {}
		virtual bool8 Show(EditorState* es) override;
	private:
		real32 frameTimes[256] = {};
		real32 minTime = REAL_MAX;
		real32 maxTime = REAL_MIN;
	};

	class EditorRoomSettingsWindow : public EditorWindow
	{
	public:
		EditorRoomSettingsWindow(Room* room) : EditorWindow("Room", true), room(room) {}
		virtual bool8 Show(EditorState* es) override;
	private:
		Room* room;
	};



	inline static String ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";

	template<typename T>
	bool ComboEnum(const String& lable, int32* currentItem)
	{
		constexpr int32 count = (int32)T::Value::COUNT;
		const char* items[count] = {};
		for (int32 i = 0; i < count; i++)
		{
			items[i] = T::__STRINGS__[i].GetCStr();
		}

		return ImGui::Combo("Type", currentItem, items, count);
	}

	template<typename T>
	ResourceId ComboBoxOfAsset(const char* label, const ManagedArray<T>& assets, ResourceId currentId)
	{
		int32 currentItem = 0;
		const char** items = GameMemory::PushTransientCount<const char*>(assets.count);
		String* itemData = GameMemory::PushTransientCount<String>(assets.count);

		for (uint32 i = 0; i < assets.count; i++)
		{
			if (assets[i].id == currentId) { currentItem = i; }
			itemData[i].Add(assets[i].name);
			items[i] = itemData[i].GetCStr();
		}

		if (ImGui::Combo(label, &currentItem, items, assets.count))
		{
			currentId = assets[currentItem].id;
		}

		return currentId;
	}
}
