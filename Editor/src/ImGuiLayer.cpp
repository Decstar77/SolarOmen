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
	static String ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";

	EditorWindowList::EditorWindowList() : EditorWindow("WINDOW LIST", true)
	{
	}

	bool8 EditorWindowList::Show()
	{
		for (uint64 i = 0; i < windows.size(); i++)
		{
			auto window = windows.at(i);

			if (window->ShouldShow())
			{
				bool8 remove = window->Show();
				if (remove)
				{
					windows.erase(windows.begin() + i);
					i--;
				}
			}
		}

		return windows.empty();
	}

	bool8 EditorWindowList::Add(std::shared_ptr<EditorWindow> window)
	{
		for (const auto& w : windows)
		{
			if (w->GetName() == window->GetName())
			{
				SOLERROR("Can not create window with same name");
				return false;
			}
		}

		windows.push_back(window);
		return true;
	}

	TextureMetaFileWindow::TextureMetaFileWindow(const String& name) : EditorWindow(name, true)
	{
		FileProcessor fileProcessor = {};
		MetaProcessor metaProcessor = {};
		metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));
		path = metaProcessor.Find(name);
		file = metaProcessor.ParseTextureMetaFile(path);
		if (!file.id.IsValid())
		{
			show = false;
			SOLERROR("Could not edit texture meta file");
		}
	}

	bool8 TextureMetaFileWindow::Show()
	{
		String windowName = "MetaData";
		windowName.Add(name);
		if (ImGui::Begin(windowName.GetCStr(), &show))
		{
			ImGui::Text("Id: %llu", file.id.number);
			ImGui::Text("Format: %s", file.format.ToString().GetCStr());
			ImGui::Checkbox("Generate Mip maps", &file.mips);
			ImGui::Checkbox("Is Skybox", &file.isSkybox);
			ImGui::Checkbox("Is Normal Map", &file.isNormalMap);

			if (ImGui::Button("Save"))
			{
				MetaProcessor metaProcessor = {};
				metaProcessor.SaveMetaData(path, file);
			}


			ImGui::End();
		}

		return !show;
	}


	bool8 EditorPerformanceWindow::Show()
	{
		if (ImGui::Begin(GetName().GetCStr(), &show))
		{
			ImGui::Text("Permanent Memory Used: %llu mbs", GameMemory::GetTheAmountOfPermanentMemoryUsed() / (1024 * 1024));
			ImGui::Text("Transient Memory Used: %llu mbs", GameMemory::GetTheAmountOfTransientMemoryUsed() / (1024 * 1024));

			ImGui::Text("Permanent Memory Allocated: %llu mbs", GameMemory::GetTheTotalAmountOfPermanentMemoryAllocated() / (1024 * 1024));
			ImGui::Text("Transient Memory Allocated: %llu mbs", GameMemory::GetTheTotalAmountOfTransientMemoryAllocated() / (1024 * 1024));

			ImGui::Separator();

			for (int32 i = 0; i < ArrayCount(frameTimes) - 1; i++)
			{
				frameTimes[i] = frameTimes[i + 1];
			}

			real32 dt = Application::GetDeltaTime();
			frameTimes[ArrayCount(frameTimes) - 1] = dt * 1000;

			minTime = Min(minTime, dt);
			maxTime = Max(maxTime, dt);

			ImGui::Text("Min %f", minTime * 1000.0f);
			ImGui::SameLine();
			ImGui::Text("Max %f", maxTime * 1000.0f);
			ImGui::SameLine();
			if (ImGui::Button("Reset")) { minTime = REAL_MAX; maxTime = REAL_MIN; }
			ImGui::PlotLines("Frame time", frameTimes, ArrayCount(frameTimes), 0, 0, 0, 30, ImVec2(128, 128), 4);


			ImGui::End();
		}

		return !show;
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
				if (ImGui::MenuItem("New")) {}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					//CString path = PlatformOpenNFileDialogAndReturnPath();
					//LoadAGameWorld(gs, rs, as, es, path);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Room Settings")) { es->showRoomWindow = true; }
				if (ImGui::MenuItem("Render Settings")) { es->showRenderSettingsWindow = true; }
				if (ImGui::MenuItem("Performance")) { es->windows.Add(std::make_shared<EditorPerformanceWindow>()); }
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
				if (ImGui::SmallButton("Edit")) { SOLINFO(res->name.GetCStr()); }
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

		es->windows.Show();
	}
}

