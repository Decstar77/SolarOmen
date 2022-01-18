#include "ImGuiLayer.h"
#include <SolarEngine.h>
#include <src/renderer/SolarRenderer.h>
#include "src/SolarEntry.h"
#include "processors/AssetPacking.h"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_win32.h"
#include "../vendor/imgui/imgui_impl_dx11.h"

#include <Windows.h>
#include "../vendor/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace sol
{
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
				if (ImGui::MenuItem("New")) {}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					//CString path = PlatformOpenNFileDialogAndReturnPath();
					//LoadAGameWorld(gs, rs, as, es, path);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Room Window")) { es->showRoomWindow = true; }
				if (ImGui::MenuItem("Render Settings")) { es->showRenderSettingsWindow = true; }
				if (ImGui::MenuItem("Performance")) { es->showPerformanceWindow = true; }
				if (ImGui::MenuItem("Console")) { es->showConsoleWindow = true; }
				if (ImGui::MenuItem("Build")) { es->showBuildWindow = true; }
				if (ImGui::MenuItem("Inspector")) { es->showInspectorWindow = true; }
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
		for (int32 i = 0; i < ArrayCount(es->frameTimes) - 1; i++)
		{
			es->frameTimes[i] = es->frameTimes[i + 1];
		}
		es->frameTimes[ArrayCount(es->frameTimes) - 1] = dt * 1000;

		es->minTime = Min(es->minTime, dt);
		es->maxTime = Max(es->maxTime, dt);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::Begin("Performance", &es->showPerformanceWindow);
		ImGui::Text("Min %f", es->minTime * 1000.0f);
		ImGui::Text("max %f", es->maxTime * 1000.0f);
		if (ImGui::Button("Reset"))
		{
			es->minTime = REAL_MAX;
			es->maxTime = REAL_MIN;
		}
		ImGui::PlotLines("Frame time", es->frameTimes, ArrayCount(es->frameTimes), 0, 0, 0, 30, ImVec2(128, 128), 4);

		ImGui::End();
		ImGui::PopStyleVar();
	}

	static void ShowAssetWindow(EditorState* es, real32 dt)
	{
		ImGui::Begin("Assets", &es->showAssetWindow);

		if (ImGui::CollapsingHeader("Models"))
		{
			auto resources = Resources::GetAllModelResources();
			ImGui::Text("Total: %i", resources.count);

			for (uint32 i = 0; i < resources.count; i++)
			{
				auto* res = &resources[i];
				ImGui::Text(res->name.GetCStr());
			}
		}

		if (ImGui::CollapsingHeader("Textures"))
		{
			auto resources = Resources::GetAllTextureResources();
			ImGui::Text("Total: %i", resources.count);

			for (uint32 i = 0; i < resources.count; i++)
			{
				auto* res = &resources[i];
				ImGui::Text(res->name.GetCStr());
			}
		}

		if (ImGui::CollapsingHeader("Shader"))
		{
			auto resources = Resources::GetAllProgramResources();
			ImGui::Text("Total: %i", resources.count);

			for (uint32 i = 0; i < resources.count; i++)
			{
				auto* res = &resources[i];
				ImGui::Text(res->name.GetCStr());
			}
		}

		static String ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";

		if (ImGui::Button("Pack Models"))
		{
			FileProcessor fileProcessor;
			MetaProcessor metaProcessor;
			metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

			auto models = LoadAndProcessModels("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Models/gltf/", fileProcessor, metaProcessor);
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

			auto textures = LoadAndProcessTextures("F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/Models/gltf/", fileProcessor, metaProcessor);
			SaveBinaryData(textures, "Assets/Packed/textures.bin");
		}


		ImGui::End();

	}

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

	static void ShowInspectorWindow(EditorState* es, real32 dt)
	{
		ImGui::Begin("Inspector", &es->showInspectorWindow);

		if (es->selectedEntity.IsValid())
		{
			MaterialComponent* materialComp = es->selectedEntity.GetMaterialomponent();
			materialComp->material.modelId = ComboBoxOfAsset("Model", Resources::GetAllModelResources(), materialComp->material.modelId);
			materialComp->material.albedoId = ComboBoxOfAsset("Abledo", Resources::GetAllTextureResources(), materialComp->material.albedoId);
			materialComp->material.programId = ComboBoxOfAsset("Program", Resources::GetAllProgramResources(), materialComp->material.programId);
		}

		ImGui::End();
	}

	void UpdateImGui(EditorState* es, real32 dt)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();

		ShowMainMenuBar(es);
		if (es->showPerformanceWindow) { ShowPerformanceWindow(es, dt); }
		if (es->showAssetWindow) { ShowAssetWindow(es, dt); }
		if (es->showInspectorWindow) { ShowInspectorWindow(es, dt); }
	}
}

