#include "ImGuiLayer.h"
#include <SolarEngine.h>
#include <src/renderer/SolarRenderer.h>
#include "src/SolarEntry.h"


#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_win32.h"
#include "../vendor/imgui/imgui_impl_dx11.h"

#include <Windows.h>
#include "../vendor/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace sol
{
	bool8 EditorSelectionAction::Redo(EditorState* es)
	{
		es->selection.Set(cur);
		return true;
	}

	bool8 EditorSelectionAction::Undo(EditorState* es)
	{
		es->selection.Set(old);
		return true;
	}

	bool8 EditorSetParentAction::Redo(EditorState* es)
	{
		entity.SetParent(newParent);
		return true;
	}

	bool8 EditorSetParentAction::Undo(EditorState* es)
	{
		entity.SetParent(oldParent);
		return true;
	}

	bool8 EditorSetNameAction::Redo(EditorState* es)
	{
		if (entity.IsValid()) {
			entity.SetName(curName);
			return true;
		}

		return false;
	}

	bool8 EditorSetNameAction::Undo(EditorState* es)
	{
		if (entity.IsValid()) {
			entity.SetName(oldName);
			return true;
		}

		return false;
	}

	static bool8 ImguiWin32MessageCallback(uint16 eventCode, void* sender, void* listener, EventContext data)
	{
		struct Win32EventPumpMessageContext
		{
			HWND hwnd;
			UINT msg;
			WPARAM wparam;
			LPARAM lparam;
		};

		Win32EventPumpMessageContext* cxt = (Win32EventPumpMessageContext*)&data;

		LRESULT result = ImGui_ImplWin32_WndProcHandler(cxt->hwnd, cxt->msg, cxt->wparam, cxt->lparam);
		if (result)
		{
			*(LRESULT*)sender = result;
			return true;
		}

		return false;
	}

	static bool8 ImguiDrawMessageCallback(uint16 eventCode, void* sender, void* listener, EventContext data)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return false;
	}

	static bool8 ImguiShutdownMessageCallback(uint16 eventCode, void* sender, void* listener, EventContext data)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		return 0;
	}

	bool8 InitialzieImGui()
	{
		if (ImGui::CreateContext())
		{
			ImGui::StyleColorsDark();
			ImGui::GetStyle().WindowRounding = 0;

			if (ImGui_ImplWin32_Init(Platform::GetNativeState()))
			{
				struct DC
				{
					ID3D11Device* device;
					ID3D11DeviceContext* context;
				};

				DC* dc = (DC*)Renderer::GetNativeDeviceContext();
				if (ImGui_ImplDX11_Init(dc->device, dc->context))
				{
					if (EventSystem::Register((uint16)EngineEvent::Value::WINDOW_PUMP_MESSAGES, nullptr, ImguiWin32MessageCallback))
					{
						if (EventSystem::Register((uint16)EngineEvent::Value::ON_RENDER_END, nullptr, ImguiDrawMessageCallback))
						{
							if (EventSystem::Register((uint16)EngineEvent::Value::ON_RENDERER_SHUTDOWN, nullptr, ImguiShutdownMessageCallback))
							{

								return true;
							}
							else
							{

							}
						}
						else
						{

						}
					}
					else
					{

					}
				}
				else
				{
					SOLFATAL("Could not start dx11 imgui -> make sure the deviceContext is the first thing in the struct");
				}
			}
			else
			{
				SOLFATAL("Could not start win32 imgui -> make sure the window handle is the first thing in the struct");
			}
		}
		else
		{

		}

		return false;
	}

	static void ShowMainMenuBar(EditorState* es)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N")) {

				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					if (!RoomProcessor::SaveRoom("Assets/Raw/Rooms/", &es->room)) {
						es->windows.Add(std::make_shared<EditorRoomSettingsWindow>(&es->room));
					}
				}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					RoomResource* res = GameMemory::PushTransientStruct<RoomResource>();
					String path = Platform::OpenNativeFileDialog();
					if (path.GetLength() > 0) {
						SOLINFO(String("Loading...").Add(path).GetCStr());
						if (RoomProcessor::ParseRoomTextFile(path, res)) {
							GameMemory::ZeroStruct<Room>(&es->room);
							if (es->room.Initliaze(res)) {

							}
						}
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Room Settings")) { es->windows.Add(std::make_shared<EditorRoomSettingsWindow>(&es->room)); }
				if (ImGui::MenuItem("Render Settings")) { es->showRenderSettingsWindow = true; }
				if (ImGui::MenuItem("Performance")) { es->windows.Add(std::make_shared<EditorPerformanceWindow>()); }
				if (ImGui::MenuItem("Console")) { es->showConsoleWindow = true; }
				if (ImGui::MenuItem("Build")) { es->showBuildWindow = true; }
				if (ImGui::MenuItem("Inspector")) { es->windows.Add(std::make_shared<EditorEntityInspectorWindow>()); }
				if (ImGui::MenuItem("Asset")) { es->showAssetWindow = true; }
				//if (ImGui::MenuItem("Main window")) { es->windowOpen = true; }
				//if (ImGui::MenuItem("Physics")) { es->physicsWindowOpen = true; }
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	static void ShowPerformanceWindow(EditorState* es, real32 dt)
	{

	}

	static void ShowAssetWindow(EditorState* es, real32 dt)
	{
		ImGui::Begin("Assets", &es->showAssetWindow);
		uint32 idCount = 0;
		String filter = "";
		if (ImGui::CollapsingHeader("Models"))
		{
			filter.Clear();
			ImGui::InputText("Model Filter", filter.GetCStr(), String::CAPCITY);
			filter.CalculateLength();
			auto resources = Resources::GetAllModelResources();
			ImGui::Separator();
			ImGui::Text("Total: %i", resources.count);
			ImGui::Separator();

			filter.ToUpperCase();
			for (uint32 i = 0; i < resources.count; i++)
			{
				auto* res = &resources[i];
				if (filter.GetLength() > 0)
				{
					String name = res->name;
					name.ToUpperCase();
					if (!name.Contains(filter))
					{
						continue;
					}
				}
				ImGui::Text(res->name.GetCStr());
				ImGui::SameLine(ImGui::GetWindowWidth() - 40);
				ImGui::PushID(idCount++);
				if (ImGui::SmallButton("Edit")) { es->windows.Add(std::make_shared<ModelMetaFileWindow>(res->name)); }
				ImGui::PopID();
			}
		}

		if (ImGui::CollapsingHeader("Textures"))
		{
			filter.Clear();
			ImGui::InputText("Texture Filter", filter.GetCStr(), String::CAPCITY);
			filter.CalculateLength();
			auto resources = Resources::GetAllTextureResources();
			ImGui::Separator();
			ImGui::Text("Total: %i", resources.count);
			ImGui::Separator();

			filter.ToUpperCase();
			for (uint32 i = 0; i < resources.count; i++)
			{
				auto* res = &resources[i];
				if (filter.GetLength() > 0)
				{
					String name = res->name;
					name.ToUpperCase();
					if (!name.Contains(filter))
					{
						continue;
					}
				}

				ImGui::Text(res->name.GetCStr());
				ImGui::SameLine(ImGui::GetWindowWidth() - 40);
				ImGui::PushID(idCount++);
				if (ImGui::SmallButton("Edit")) { es->windows.Add(std::make_shared<TextureMetaFileWindow>(res->name)); }
				ImGui::PopID();
			}
		}

		if (ImGui::CollapsingHeader("Programs"))
		{
			filter.Clear();
			ImGui::InputText("Program Filter", filter.GetCStr(), String::CAPCITY);
			filter.CalculateLength();
			auto resources = Resources::GetAllProgramResources();
			ImGui::Separator();
			ImGui::Text("Total: %i", resources.count);
			ImGui::Separator();

			filter.ToUpperCase();
			for (uint32 i = 0; i < resources.count; i++)
			{
				auto* res = &resources[i];
				if (filter.GetLength() > 0)
				{
					String name = res->name;
					name.ToUpperCase();
					if (!name.Contains(filter))
					{
						continue;
					}
				}

				ImGui::Text(res->name.GetCStr());
			}
		}



		if (ImGui::Button("Pack Models"))
		{
			FileProcessor fileProcessor;
			MetaProcessor metaProcessor;
			metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

			auto models = LoadAndProcessModels("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Models/obj/", fileProcessor, metaProcessor);
			SaveBinaryData(models, "Assets/Packed/models.bin");
		}

		ImGui::SameLine();

		if (ImGui::Button("Reload Models"))
		{
			Resources::LoadAllModelResources();
			Renderer::LoadAllModels();
		}

		if (ImGui::Button("Pack Textures"))
		{
			FileProcessor fileProcessor;
			MetaProcessor metaProcessor;
			metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

			//auto textures = LoadAndProcessTextures("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Models/obj/", fileProcessor, metaProcessor);
			auto textures = LoadAndProcessTextures("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Skyboxes/", fileProcessor, metaProcessor);
			SaveBinaryData(textures, "Assets/Packed/textures.bin");
		}


		ImGui::End();

	}

	void UpdateImGui(EditorState* es, real32 dt)
	{
		Input* input = Input::Get();
		if (IsKeyJustDown(input, s) && input->ctrl) {
			if (!RoomProcessor::SaveRoom("Assets/Raw/Rooms/", &es->room)) {
				es->windows.Add(std::make_shared<EditorRoomSettingsWindow>(&es->room));
			}
		}

		//if (IsKeyJustDown(input, del)) {
		//	for (Entity entity : es->selectedEntities)
		//	{
		//		es->room.DestoryEntity(&entity);
		//	}
		//	es->selectedEntities.clear();
		//}


		if (IsKeyJustDown(input, z) && input->ctrl && input->shift) {
			es->undoSystem.Redo(es);
		}
		else if (IsKeyJustDown(input, z) && input->ctrl) {
			es->undoSystem.Undo(es);
		}


		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();

		ShowMainMenuBar(es);
		if (es->showPerformanceWindow) { ShowPerformanceWindow(es, dt); }
		if (es->showAssetWindow) { ShowAssetWindow(es, dt); }

		es->windows.Show(es);
	}
}

